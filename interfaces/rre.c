/* RETRO --------------------------------------------------------------
  A personal, minimalistic forth
  Copyright (c) 2016 - 2018 Charles Childers

  This is `rre`, short for `run retro and exit`. It's the basic
  interface layer for Retro on FreeBSD, Linux and macOS.

  rre embeds the image file into the binary, so the compiled version
  of this is all you need to have a functional system.

  I'll include commentary throughout the source, so read on.
  ---------------------------------------------------------------------*/


/*---------------------------------------------------------------------
  RRE provides numerous extensions to RETRO. Support for these can be
  enabled/disabled here. (Note that this won't remove the words, just
  support for them. You should also edit `rre.forth` to remove anything
  you don't want/need after making changes here).
  ---------------------------------------------------------------------*/

#define ENABLE_FILES
#define ENABLE_FLOATING_POINT
#define ENABLE_UNIX
#define ENABLE_GOPHER
#define USE_TERMIOS

/*---------------------------------------------------------------------
  Begin by including the various C headers needed.
  ---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

#ifdef USE_TERMIOS
#include <termios.h>
#include <sys/ioctl.h>
#endif

#define NUM_DEVICES  7

typedef void (*Handler)(void);

Handler IO_deviceHandlers[NUM_DEVICES + 1];
Handler IO_queryHandlers[NUM_DEVICES + 1];

/*---------------------------------------------------------------------
  First, a few constants relating to the image format and memory
  layout. If you modify the kernel (Rx.md), these will need to be
  altered to match your memory layout.
  ---------------------------------------------------------------------*/

#define TIB            1025
#define D_OFFSET_LINK     0
#define D_OFFSET_XT       1
#define D_OFFSET_CLASS    2
#define D_OFFSET_NAME     3


/*---------------------------------------------------------------------
  Next we get into some things that relate to the Nga virtual machine
  that RETRO runs on.
  ---------------------------------------------------------------------*/

#define CELL         int32_t      /* Cell size (32 bit, signed integer */
#define IMAGE_SIZE   524288 * 48  /* Amount of RAM. 12MiB by default.  */
#define ADDRESSES    2048         /* Depth of address stack            */
#define STACK_DEPTH  512          /* Depth of data stack               */

CELL sp, rp, ip;                  /* Data, address, instruction pointers */
CELL data[STACK_DEPTH];           /* The data stack                    */
CELL address[ADDRESSES];          /* The address stack                 */
CELL memory[IMAGE_SIZE + 1];      /* The memory for the image          */

#define TOS  data[sp]             /* Shortcut for top item on stack    */
#define NOS  data[sp-1]           /* Shortcut for second item on stack */
#define TORS address[rp]          /* Shortcut for top item on address stack */


/*---------------------------------------------------------------------
  RRE embeds the image into the binary. This includes the image data
  (converted to a .c file by an external tool).
  ---------------------------------------------------------------------*/

#include "rre_image_unix.c"


/*---------------------------------------------------------------------
  Moving forward, a few variables. These are updated to point to the
  latest values in the image.
  ---------------------------------------------------------------------*/

CELL Dictionary;
CELL NotFound;
CELL interpret;


/*---------------------------------------------------------------------
  Function prototypes.
  ---------------------------------------------------------------------*/

CELL stack_pop();
void stack_push(CELL value);
CELL string_inject(char *str, CELL buffer);
char *string_extract(CELL at);
CELL d_link(CELL dt);
CELL d_xt(CELL dt);
CELL d_class(CELL dt);
CELL d_name(CELL dt);
CELL d_lookup(CELL Dictionary, char *name);
CELL d_xt_for(char *Name, CELL Dictionary);
void update_rx();
void execute(CELL cell, int silent);
void evaluate(char *s, int silent);
int not_eol(int ch);
void read_token(FILE *file, char *token_buffer, int echo);
void include_file(char *fname, int run_tests);
CELL ngaLoadImage(char *imageFile);
void ngaPrepare();
void ngaProcessOpcode(CELL opcode);
void ngaProcessPackedOpcodes(CELL opcode);
int ngaValidatePackedOpcodes(CELL opcode);


/*---------------------------------------------------------------------
  Declare global variables related to I/O.
  ---------------------------------------------------------------------*/

char **sys_argv;
int sys_argc;
int silence_input;

/*---------------------------------------------------------------------
  Now to the fun stuff: interfacing with the virtual machine. There are
  a things I like to have here:

  - push a value to the stack
  - pop a value off the stack
  - extract a string from the image
  - inject a string into the image.
  - lookup dictionary headers and access dictionary fields
  ---------------------------------------------------------------------*/


/*---------------------------------------------------------------------
  Stack push/pop is easy. I could avoid these, but it aids in keeping
  the code readable, so it's worth the slight overhead.
  ---------------------------------------------------------------------*/

CELL stack_pop() {
  sp--;
  if (sp < 0) {
    printf("Data stack underflow.\n");
    exit(1);
  }
  return data[sp + 1];
}

void stack_push(CELL value) {
  sp++;
  if (sp >= STACK_DEPTH) {
    printf("Data stack overflow.\n");
    exit(1);
  }
  data[sp] = value;
}


/*---------------------------------------------------------------------
  Strings are next. RETRO uses C-style NULL terminated strings. So I
  can easily inject or extract a string. Injection iterates over the
  string, copying it into the image. This also takes care to ensure
  that the NULL terminator is added.
  ---------------------------------------------------------------------*/

CELL string_inject(char *str, CELL buffer) {
  CELL m, i;
  if (!str) {
    memory[buffer] = 0;
    return 0;
  }
  m = strlen(str);
  i = 0;
  while (m > 0) {
    memory[buffer + i] = (CELL)str[i];
    memory[buffer + i + 1] = 0;
    m--; i++;
  }
  return buffer;
}


/*---------------------------------------------------------------------
  Extracting a string is similar, but I have to iterate over the VM
  memory instead of a C string and copy the charaters into a buffer.
  This uses a static buffer (`string_data`) as I prefer to avoid using
  `malloc()`.
  ---------------------------------------------------------------------*/

char string_data[8192];
char *string_extract(CELL at) {
  CELL starting = at;
  CELL i = 0;
  while(memory[starting] && i < 8192)
    string_data[i++] = (char)memory[starting++];
  string_data[i] = 0;
  return (char *)string_data;
}


/*---------------------------------------------------------------------
  Continuing along, I now define functions to access the dictionary.

  RETRO's dictionary is a linked list. Each entry is setup like:

  0000  Link to previous entry (NULL if this is the root entry)
  0001  Pointer to definition start
  0002  Pointer to class handler
  0003  Start of a NULL terminated string with the word name

  First, functions to access each field. The offsets were defineed at
  the start of the file.
  ---------------------------------------------------------------------*/

CELL d_link(CELL dt) {
  return dt + D_OFFSET_LINK;
}

CELL d_xt(CELL dt) {
  return dt + D_OFFSET_XT;
}

CELL d_class(CELL dt) {
  return dt + D_OFFSET_CLASS;
}

CELL d_name(CELL dt) {
  return dt + D_OFFSET_NAME;
}


/*---------------------------------------------------------------------
  Next, a more complext word. This will walk through the entries to
  find one with a name that matches the specified name. This is *slow*,
  but works ok unless you have a really large dictionary. (I've not
  run into issues with this in practice).
  ---------------------------------------------------------------------*/

CELL d_lookup(CELL Dictionary, char *name) {
  CELL dt = 0;
  CELL i = Dictionary;
  char *dname;
  while (memory[i] != 0 && i != 0) {
    dname = string_extract(d_name(i));
    if (strcmp(dname, name) == 0) {
      dt = i;
      i = 0;
    } else {
      i = memory[i];
    }
  }
  return dt;
}


/*---------------------------------------------------------------------
  My last dictionary related word returns the `xt` pointer for a word.
  This is used to help keep various important bits up to date.
  ---------------------------------------------------------------------*/

CELL d_xt_for(char *Name, CELL Dictionary) {
  return memory[d_xt(d_lookup(Dictionary, Name))];
}


/*---------------------------------------------------------------------
  This interface tracks a few words and variables in the image. These
  are:

  Dictionary - the latest dictionary header
  NotFound   - called when a word is not found
  interpret  - the heart of the interpreter/compiler

  I have to call this periodically, as the Dictionary will change as
  new words are defined, and the user might write a new error handler
  or interpreter.
  ---------------------------------------------------------------------*/

void update_rx() {
  Dictionary = memory[2];
  interpret = d_xt_for("interpret", Dictionary);
  NotFound = d_xt_for("err:notfound", Dictionary);
}


/*---------------------------------------------------------------------
  Now on to I/O and extensions!

  RRE provides a lot of additional functionality over the base RETRO
  system. First up is support for files.

  The RRE file model is intended to be similar to that of the standard
  C libraries and wraps fopen(), fclose(), etc.
  ---------------------------------------------------------------------*/

void generic_output() {
  putc(stack_pop(), stdout);
  fflush(stdout);
}

void generic_output_query() {
  stack_push(0);
  stack_push(0);
}

void io_keyboard_handler() {
  stack_push(getc(stdin));
  if (TOS == 127) TOS = 8;
#ifdef USE_TERMIOS
  if (silence_input != -1) {
    putc(TOS, stdout);
    fflush(stdout);
  }
#endif
}

void io_keyboard_query() {
  stack_push(0);
  stack_push(1);
}

#ifdef ENABLE_FILES
void io_filesystem_query();
void io_filesystem_handler();
#endif


/*---------------------------------------------------------------------
  UNIX. Or Linux. Or BSD. Or whatever. This section adds functions for
  interacting with the host OS. It's tested on FreeBSD and Linux, but
  likely won't work on Windows or any other host not supporting POSIX.
  ---------------------------------------------------------------------*/

#ifdef ENABLE_UNIX
void io_unix_query();
void io_unix_handler();
#endif


/*---------------------------------------------------------------------
  Floating Point.

  I kind of dislike floating point (it's tricky, and not as precise as
  fixed point), but it is useful for certain things. I include support
  for it as it's been requested frequently by my users.
  ---------------------------------------------------------------------*/

#ifdef ENABLE_FLOATING_POINT
void io_floatingpoint_query();
void io_floatingpoint_handler();
#endif


/*---------------------------------------------------------------------
  Gopher Support

  I'm a big fan of Gopher, so RRE provides support for fetching files
  via the Gopher protocol.
  ---------------------------------------------------------------------*/

#ifdef ENABLE_GOPHER
void io_gopher_query();
void io_gopher_handler();
#endif


/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/

#ifdef USE_TERMIOS
struct termios new_termios, old_termios;

void prepare_term() {
  tcgetattr(0, &old_termios);
  new_termios = old_termios;
  new_termios.c_iflag &= ~(BRKINT+ISTRIP+IXON+IXOFF);
  new_termios.c_iflag |= (IGNBRK+IGNPAR);
  new_termios.c_lflag &= ~(ICANON+ISIG+IEXTEN+ECHO);
  new_termios.c_cc[VMIN] = 1;
  new_termios.c_cc[VTIME] = 0;
  tcsetattr(0, TCSANOW, &new_termios);
}

void restore_term() {
  tcsetattr(0, TCSANOW, &old_termios);
}
#endif


void scripting_arg() {
  CELL a, b;
  a = stack_pop();
  b = stack_pop();
  stack_push(string_inject(sys_argv[a + 2], b));
}

void scripting_arg_count() {
  stack_push(sys_argc - 2);
}

void scripting_include() {
  include_file(string_extract(stack_pop()), 0);
}

Handler ScriptingActions[] = {
  scripting_arg,
  scripting_arg_count,
  scripting_include
};

void io_scripting_query() {
  stack_push(0);
  stack_push(9);
}

void io_scripting_handler() {
  ScriptingActions[stack_pop()]();
}

/*---------------------------------------------------------------------
  With these out of the way, I implement `execute`, which takes an
  address and runs the code at it. This has a couple of interesting
  bits.

  Nga uses packed instruction bundles, with up to four instructions per
  bundle. Since RETRO requires an additional instruction to handle
  displaying a character, I define the handler for that here.

  This will also exit if the address stack depth is zero (meaning that
  the word being run, and it's dependencies) are finished.
  ---------------------------------------------------------------------*/

void execute(CELL cell, int silent) {
  CELL a, b, token;
  CELL opcode;
  silence_input = silent;
  rp = 1;
  ip = cell;
  token = TIB;
  while (ip < IMAGE_SIZE) {
    if (ip == NotFound) {
      printf("\nERROR: Word Not Found: ");
      printf("`%s`\n\n", string_extract(token));
    }
    if (ip == interpret) {
      token = TOS;
    }
    opcode = memory[ip];
    if (ngaValidatePackedOpcodes(opcode) != 0) {
      ngaProcessPackedOpcodes(opcode);
    } else {
      printf("Invalid instruction!\n");
      printf("At %d, opcode %d\n", ip, opcode);
#ifdef USE_TERMIOS
      restore_term();
#endif
      exit(1);
    }
    ip++;
    if (rp == 0)
      ip = IMAGE_SIZE;
  }
}



/*---------------------------------------------------------------------
  RETRO's `interpret` word expects a token on the stack. This next
  function copies a token to the `TIB` (text input buffer) and then
  calls `interpret` to process it.
  ---------------------------------------------------------------------*/

void evaluate(char *s, int silent) {
  if (strlen(s) == 0)
    return;
  update_rx();
  string_inject(s, TIB);
  stack_push(TIB);
  execute(interpret, silent);
}


/*---------------------------------------------------------------------
  `read_token` reads a token from the specified file.  It will stop on
   a whitespace or newline. It also tries to handle backspaces, though
   the success of this depends on how your terminal is configured.
  ---------------------------------------------------------------------*/

int not_eol(int ch) {
  return (ch != (char)10) && (ch != (char)13) && (ch != (char)32) && (ch != EOF) && (ch != 0);
}

void read_token(FILE *file, char *token_buffer, int echo) {
  int ch = getc(file);
  int count = 0;
  if (echo != 0)
    putchar(ch);
  while (not_eol(ch))
  {
    if ((ch == 8 || ch == 127) && count > 0) {
      count--;
      if (echo != 0) {
        putchar(8);
        putchar(32);
        putchar(8);
      }
    } else {
      token_buffer[count++] = ch;
    }
    ch = getc(file);
    if (echo != 0)
      putchar(ch);
  }
  token_buffer[count] = '\0';
}


/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/
void dump_stack() {
  CELL i;
  if (sp == 0)
    return;
  printf("\nStack: ");
  for (i = 1; i <= sp; i++) {
    if (i == sp)
      printf("[ TOS: %d ]", data[i]);
    else
      printf("%d ", data[i]);
  }
  printf("\n");
}


/*---------------------------------------------------------------------
  RRE is primarily intended to be used in a batch or scripting model.
  The `include_file()` function will be used to read the code in the
  file, evaluating it as encountered.

  I enforce a literate model, with code in fenced blocks. E.g.,

    # This is a test

    Display "Hello, World!"

    ~~~
    'Hello,_World! puts nl
    ~~~

  RRE will ignore anything outside the `~~~` blocks. To identify if the
  current token is the start or end of a block, I provide a `fenced()`
  function.
  ---------------------------------------------------------------------*/

int fenced(char *s)
{
  int a = strcmp(s, "```");
  int b = strcmp(s, "~~~");
  if (a == 0) return 2;
  if (b == 0) return 1;
              return 0;
}


/*---------------------------------------------------------------------
  And now for the actual `include_file()` function.
  ---------------------------------------------------------------------*/

void include_file(char *fname, int run_tests) {
  int inBlock = 0;                 /* Tracks status of in/out of block */
  char source[64 * 1024];          /* Line buffer [about 64K]          */
  char fence[4];                   /* Used with `fenced()`             */

  FILE *fp;                        /* Open the file. If not found,     */
  fp = fopen(fname, "r");          /* exit.                            */
  if (fp == NULL)
    return;

  while (!feof(fp))                /* Loop through the file            */
  {
    read_token(fp, source, 0);
    strncpy(fence, source, 3);     /* Copy the first three characters  */
    fence[3] = '\0';               /* into `fence` to see if we are in */
    if (fenced(fence) > 0) {       /* a code block.                    */
      if (fenced(fence) == 2 && run_tests == 0) {
      } else {
        if (inBlock == 0)
          inBlock = 1;
        else
          inBlock = 0;
      }
    } else {
      if (inBlock == 1)            /* If we are, evaluate token        */
        evaluate(source, -1);
    }
  }

  fclose(fp);
}


/*---------------------------------------------------------------------
  `help()` displays a summary of the command line arguments RRE allows.

  This is invoked using `rre -h`
  ---------------------------------------------------------------------*/

void help(char *exename) {
  printf("Scripting Usage: %s filename\n\n", exename);
  printf("Interactive Usage: %s [-h] [-i] [-c] [-s] [-f filename] [-t]\n\n", exename);
  printf("Valid Arguments:\n\n");
  printf("  -h\n");
  printf("  Display this help text\n\n");
  printf("  -i\n");
  printf("  Launches in interactive mode (line buffered)\n\n");
  printf("  -c\n");
  printf("  Launches in interactive mode (character buffered)\n\n");
  printf("  -s\n");
  printf("  Suppress the 'ok' prompt and keyboard echo in interactive mode\n\n");
  printf("  -f filename\n");
  printf("  Run the contents of the specified file\n\n");
  printf("  -t\n");
  printf("  Run tests (in ``` blocks) in any loaded files\n\n");
}


/*---------------------------------------------------------------------
  `initialize()` sets up Nga and loads the image (from the array in
  `image.c`) to memory.
  ---------------------------------------------------------------------*/

void initialize() {
  CELL i;
  ngaPrepare();
  for (i = 0; i < ngaImageCells; i++)
    memory[i] = ngaImage[i];
  update_rx();
}


/*---------------------------------------------------------------------
  `arg_is()` exists to aid in readability. It compares the first actual
  command line argument to a string and returns a boolean flag.
  ---------------------------------------------------------------------*/

int arg_is(char *t) {
  return strcmp(sys_argv[1], t) == 0;
}


/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/

enum flags {
  FLAG_HELP, FLAG_RUN_TESTS, FLAG_INCLUDE, FLAG_INTERACTIVE, FLAG_CBREAK, FLAG_SILENT
};

int main(int argc, char **argv) {
  int i;
  int modes[32];
  char *files[16];
  int fsp;

  int run_tests;

  if (argc <= 1) return 0;                /* Guard clause: exit if no  */
                                          /* arguments are passed.     */

  initialize();                           /* Initialize Nga & image    */
  IO_deviceHandlers[0] = generic_output;
  IO_queryHandlers[0] = generic_output_query;
  IO_deviceHandlers[1] = io_keyboard_handler;
  IO_queryHandlers[1] = io_keyboard_query;
  IO_deviceHandlers[2] = io_filesystem_handler;
  IO_queryHandlers[2] = io_filesystem_query;
  IO_deviceHandlers[3] = io_gopher_handler;
  IO_queryHandlers[3] = io_gopher_query;
  IO_deviceHandlers[4] = io_floatingpoint_handler;
  IO_queryHandlers[4] = io_floatingpoint_query;
  IO_deviceHandlers[5] = io_unix_handler;
  IO_queryHandlers[5] = io_unix_query;
  IO_deviceHandlers[6] = io_scripting_handler;
  IO_queryHandlers[6] = io_scripting_query;

  sys_argc = argc;                        /* Point the global argc and */
  sys_argv = argv;                        /* argv to the actual ones   */

  if (argc >= 2 && argv[1][0] != '-') {
    include_file(argv[1], 0);             /* If no flags were passed,  */
    if (sp >= 1)  dump_stack();           /* load the file specified,  */
    exit(0);                              /* and exit                  */
  }

  for (i = 0; i < 32; i++)
    modes[i] = 0;

  for (i = 0; i < 16; i++)
    files[i] = "\0";

  run_tests = 0;
  fsp = 0;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      help(argv[0]);
      exit(0);    
    } else if (strcmp(argv[i], "-i") == 0) {
      modes[FLAG_INTERACTIVE] = 1;
    } else if (strcmp(argv[i], "-c") == 0) {
      modes[FLAG_INTERACTIVE] = 1;
      modes[FLAG_CBREAK] = 1;
    } else if (strcmp(argv[i], "-s") == 0) {
      modes[FLAG_SILENT] = 1;
    } else if (strcmp(argv[i], "-f") == 0) {
      files[fsp] = argv[i + 1];
      fsp++;
      i++;
    } else if (strcmp(argv[i], "-t") == 0) {
      modes[FLAG_RUN_TESTS] = 1;
      run_tests = 1;
    }
  }

  for (i = 0; i < fsp; i++) {
    if (strcmp(files[i], "\0") != 0)
      include_file(files[i], run_tests);
  }


  if (modes[FLAG_SILENT] == 1) {
    memory[d_xt_for("NoEcho", Dictionary)] = -1;
  }

  if (modes[FLAG_INTERACTIVE] == 1) {
    execute(d_xt_for("banner", Dictionary), 0);
#ifdef USE_TERMIOS
    if (modes[FLAG_CBREAK] == 1) prepare_term();
#endif
    if (modes[FLAG_CBREAK] == 1) while (1) execute(d_xt_for("listen", Dictionary), 0);
    if (modes[FLAG_CBREAK] == 0) while (1) execute(d_xt_for("listen", Dictionary), -1);
#ifdef USE_TERMIOS
    if (modes[FLAG_CBREAK] == 1) restore_term();
#endif
    exit(0);
  }
}


/* Nga ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Copyright (c) 2008 - 2018, Charles Childers
   Copyright (c) 2009 - 2010, Luke Parrish
   Copyright (c) 2010,        Marc Simpson
   Copyright (c) 2010,        Jay Skeer
   Copyright (c) 2011,        Kenneth Keating
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

enum vm_opcode {
  VM_NOP,  VM_LIT,    VM_DUP,   VM_DROP,    VM_SWAP,   VM_PUSH,  VM_POP,
  VM_JUMP, VM_CALL,   VM_CCALL, VM_RETURN,  VM_EQ,     VM_NEQ,   VM_LT,
  VM_GT,   VM_FETCH,  VM_STORE, VM_ADD,     VM_SUB,    VM_MUL,   VM_DIVMOD,
  VM_AND,  VM_OR,     VM_XOR,   VM_SHIFT,   VM_ZRET,   VM_END,   VM_IE,
  VM_IQ,   VM_II
};
#define NUM_OPS VM_II + 1

#ifndef NUM_DEVICES
#define NUM_DEVICES 0
#endif

//Handler IO_deviceHandlers[NUM_DEVICES + 1];
//Handler IO_queryHandlers[NUM_DEVICES + 1];

CELL ngaLoadImage(char *imageFile) {
  FILE *fp;
  CELL imageSize;
  long fileLen;
  CELL i;
  if ((fp = fopen(imageFile, "rb")) != NULL) {
    /* Determine length (in cells) */
    fseek(fp, 0, SEEK_END);
    fileLen = ftell(fp) / sizeof(CELL);
    rewind(fp);
    /* Read the file into memory */
    imageSize = fread(&memory, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
  else {
    for (i = 0; i < ngaImageCells; i++)
      memory[i] = ngaImage[i];
    imageSize = i;
  }
  return imageSize;
}

void ngaPrepare() {
  ip = sp = rp = 0;
  for (ip = 0; ip < IMAGE_SIZE; ip++)
    memory[ip] = VM_NOP;
  for (ip = 0; ip < STACK_DEPTH; ip++)
    data[ip] = 0;
  for (ip = 0; ip < ADDRESSES; ip++)
    address[ip] = 0;
}

void inst_nop() {
}

void inst_lit() {
  sp++;
  ip++;
  TOS = memory[ip];
}

void inst_dup() {
  sp++;
  data[sp] = NOS;
}

void inst_drop() {
  data[sp] = 0;
   if (--sp < 0)
     ip = IMAGE_SIZE;
}

void inst_swap() {
  CELL a;
  a = TOS;
  TOS = NOS;
  NOS = a;
}

void inst_push() {
  rp++;
  TORS = TOS;
  inst_drop();
}

void inst_pop() {
  sp++;
  TOS = TORS;
  rp--;
}

void inst_jump() {
  ip = TOS - 1;
  inst_drop();
}

void inst_call() {
  rp++;
  TORS = ip;
  ip = TOS - 1;
  inst_drop();
}

void inst_ccall() {
  CELL a, b;
  a = TOS; inst_drop();  /* False */
  b = TOS; inst_drop();  /* Flag  */
  if (b != 0) {
    rp++;
    TORS = ip;
    ip = a - 1;
  }
}

void inst_return() {
  ip = TORS;
  rp--;
}

void inst_eq() {
  NOS = (NOS == TOS) ? -1 : 0;
  inst_drop();
}

void inst_neq() {
  NOS = (NOS != TOS) ? -1 : 0;
  inst_drop();
}

void inst_lt() {
  NOS = (NOS < TOS) ? -1 : 0;
  inst_drop();
}

void inst_gt() {
  NOS = (NOS > TOS) ? -1 : 0;
  inst_drop();
}

void inst_fetch() {
  switch (TOS) {
    case -1: TOS = sp - 1; break;
    case -2: TOS = rp; break;
    case -3: TOS = IMAGE_SIZE; break;
    default: TOS = memory[TOS]; break;
  }
}

void inst_store() {
  if (TOS <= IMAGE_SIZE && TOS >= 0) {
    memory[TOS] = NOS;
    inst_drop();
    inst_drop();
  } else {
    ip = IMAGE_SIZE;
  }
}

void inst_add() {
  NOS += TOS;
  inst_drop();
}

void inst_sub() {
  NOS -= TOS;
  inst_drop();
}

void inst_mul() {
  NOS *= TOS;
  inst_drop();
}

void inst_divmod() {
  CELL a, b;
  a = TOS;
  b = NOS;
  TOS = b / a;
  NOS = b % a;
}

void inst_and() {
  NOS = TOS & NOS;
  inst_drop();
}

void inst_or() {
  NOS = TOS | NOS;
  inst_drop();
}

void inst_xor() {
  NOS = TOS ^ NOS;
  inst_drop();
}

void inst_shift() {
  CELL y = TOS;
  CELL x = NOS;
  if (TOS < 0)
    NOS = NOS << (TOS * -1);
  else {
    if (x < 0 && y > 0)
      NOS = x >> y | ~(~0U >> y);
    else
      NOS = x >> y;
  }
  inst_drop();
}

void inst_zret() {
  if (TOS == 0) {
    inst_drop();
    ip = TORS;
    rp--;
  }
}

void inst_end() {
  ip = IMAGE_SIZE;
}

void inst_ie() {
  sp++;
  TOS = NUM_DEVICES;
}

void inst_iq() {
  CELL Device = TOS;
  inst_drop();
  IO_queryHandlers[Device]();
}

void inst_ii() {
  CELL Device = TOS;
  inst_drop();
  IO_deviceHandlers[Device]();
}

Handler instructions[NUM_OPS] = {
  inst_nop, inst_lit, inst_dup, inst_drop, inst_swap, inst_push, inst_pop,
  inst_jump, inst_call, inst_ccall, inst_return, inst_eq, inst_neq, inst_lt,
  inst_gt, inst_fetch, inst_store, inst_add, inst_sub, inst_mul, inst_divmod,
  inst_and, inst_or, inst_xor, inst_shift, inst_zret, inst_end, inst_ie,
  inst_iq, inst_ii
};

void ngaProcessOpcode(CELL opcode) {
  if (opcode != 0)
    instructions[opcode]();
}

int ngaValidatePackedOpcodes(CELL opcode) {
  CELL raw = opcode;
  CELL current;
  int valid = -1;
  int i;
  for (i = 0; i < 4; i++) {
    current = raw & 0xFF;
    if (!(current >= 0 && current <= 29))
      valid = 0;
    raw = raw >> 8;
  }
  return valid;
}

void ngaProcessPackedOpcodes(CELL opcode) {
  CELL raw = opcode;
  int i;
  for (i = 0; i < 4; i++) {
    ngaProcessOpcode(raw & 0xFF);
    raw = raw >> 8;
  }
}
