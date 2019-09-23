/*---------------------------------------------------------------------
  RETRO is a personal, minimalistic forth with a pragmatic focus

  This implements Nga, the virtual machine at the heart of RETRO. It
  includes a number of I/O interfaces, extensive commentary, and has
  been refined by over a decade of use and development.

  Copyright (c) 2008 - 2019, Charles Childers

  Portions are based on Ngaro, which was additionally copyrighted by
  the following:

  Copyright (c) 2009 - 2010, Luke Parrish
  Copyright (c) 2010,        Marc Simpson
  Copyright (c) 2010,        Jay Skeer
  Copyright (c) 2011,        Kenneth Keating
  ---------------------------------------------------------------------*/


/*---------------------------------------------------------------------
  C Headers
  ---------------------------------------------------------------------*/

#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>


/*---------------------------------------------------------------------
  Configuration
  ---------------------------------------------------------------------*/

#ifndef BIT64
#define CELL int32_t
#define CELL_MIN INT_MIN + 1
#define CELL_MAX INT_MAX - 1
#else
#define CELL int64_t
#define CELL_MIN LLONG_MIN + 1
#define CELL_MAX LLONG_MAX - 1
#endif

#define IMAGE_SIZE   524288 * 2   /* Amount of RAM. 4MiB (1M) cells    */
#define ADDRESSES    256          /* Depth of address stack            */
#define STACK_DEPTH  256          /* Depth of data stack               */

#define TIB            1025       /* Location of TIB                   */

#define D_OFFSET_LINK     0       /* Dictionary Format Info. Update if */
#define D_OFFSET_XT       1       /* you change the dictionary fields. */
#define D_OFFSET_CLASS    2
#define D_OFFSET_NAME     3

#define NUM_DEVICES       8       /* Set the number of I/O devices     */

#define MAX_OPEN_FILES  128


/*---------------------------------------------------------------------
  Image, Stack, and VM variables
  ---------------------------------------------------------------------*/

CELL sp, rp, ip;                  /* Stack & instruction pointers      */
CELL data[STACK_DEPTH];           /* The data stack                    */
CELL address[ADDRESSES];          /* The address stack                 */
CELL memory[IMAGE_SIZE + 1];      /* The memory for the image          */

#define TOS  data[sp]             /* Shortcut for top item on stack    */
#define NOS  data[sp-1]           /* Shortcut for second item on stack */
#define TORS address[rp]          /* Shortcut for top item on address stack */


/*---------------------------------------------------------------------
  Function Prototypes
  ---------------------------------------------------------------------*/

CELL stack_pop();
void stack_push(CELL value);
CELL string_inject(char *str, CELL buffer);
char *string_extract(CELL at);
CELL d_xt_for(char *Name, CELL Dictionary);
void update_rx();
void ngaProcessPackedOpcodes(CELL opcode);
int ngaValidatePackedOpcodes(CELL opcode);
void include_file(char *fname, int run_tests);
CELL ngaLoadImage(char *imageFile);
void ngaPrepare();
void io_output_handler();
void io_output_query();
void io_keyboard_handler();
void io_keyboard_query();
void io_filesystem_query();
void io_filesystem_handler();
void io_unix_query();
void io_unix_handler();
void io_floatingpoint_query();
void io_floatingpoint_handler();
void io_scripting_handler();
void io_scripting_query();
void io_image();
void io_image_query();

void io_socket();
void query_socket();

size_t bsd_strlcat(char *dst, const char *src, size_t dsize);
size_t bsd_strlcpy(char *dst, const char *src, size_t dsize);


/*---------------------------------------------------------------------
  Populate The I/O Device Tables
  ---------------------------------------------------------------------*/

typedef void (*Handler)(void);

Handler IO_deviceHandlers[NUM_DEVICES + 1] = {
  io_output_handler,
  io_keyboard_handler,
  io_filesystem_handler,
  io_floatingpoint_handler,
  io_scripting_handler,
  io_unix_handler,
  io_image,
  io_socket
};

Handler IO_queryHandlers[NUM_DEVICES + 1] = {
  io_output_query,
  io_keyboard_query,
  io_filesystem_query,
  io_floatingpoint_query,
  io_scripting_query,
  io_unix_query,
  io_image_query,
  query_socket
};


/*---------------------------------------------------------------------
  Variables Related To Image Introspection
  ---------------------------------------------------------------------*/

CELL Compiler;
CELL Dictionary;
CELL NotFound;
CELL interpret;


/*---------------------------------------------------------------------
  Embed The Image
  ---------------------------------------------------------------------*/

#include "retro-image.c"


/*---------------------------------------------------------------------
  Global Variables
  ---------------------------------------------------------------------*/

char string_data[8192];
char **sys_argv;
int sys_argc;
int silence_input;


/*=====================================================================*/


/*---------------------------------------------------------------------
  Now on to I/O and extensions!

  RRE provides a lot of additional functionality over the base RETRO
  system. First up is support for files.

  The RRE file model is intended to be similar to that of the standard
  C libraries and wraps fopen(), fclose(), etc.
  ---------------------------------------------------------------------*/

void io_output_handler() {
  putc(stack_pop(), stdout);
  fflush(stdout);
}

void io_output_query() {
  stack_push(0);
  stack_push(0);
}


/*=====================================================================*/


void io_keyboard_handler() {
  stack_push(getc(stdin));
  if (TOS == 127) TOS = 8;
}

void io_keyboard_query() {
  stack_push(0);
  stack_push(1);
}


/*=====================================================================*/


/*---------------------------------------------------------------------
  Scripting Support
  ---------------------------------------------------------------------*/

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

void scripting_name() {
  stack_push(string_inject(sys_argv[1], stack_pop()));
}

Handler ScriptingActions[] = {
  scripting_arg_count,
  scripting_arg,
  scripting_include,
  scripting_name
};

void io_scripting_query() {
  stack_push(0);
  stack_push(9);
}

void io_scripting_handler() {
  ScriptingActions[stack_pop()]();
}


/*=====================================================================*/


/*---------------------------------------------------------------------
  Implement Image Saving
  ---------------------------------------------------------------------*/

void io_image() {
  FILE *fp;
  char *f = string_extract(stack_pop());
  if ((fp = fopen(f, "wb")) == NULL) {
    printf("\nERROR (nga/io_image): Unable to save the image: %s!\n", f);
    exit(2);
  }
  fwrite(&memory, sizeof(CELL), memory[3] + 1, fp);
  fclose(fp);
}

void io_image_query() {
  stack_push(0);
  stack_push(1000);
}


/*=====================================================================*/


/*---------------------------------------------------------------------
  With these out of the way, I implement `execute`, which takes an
  address and runs the code at it. This has a couple of interesting
  bits.

  This will also exit if the address stack depth is zero (meaning that
  the word being run, and it's dependencies) are finished.
  ---------------------------------------------------------------------*/

void rre_execute(CELL cell, int silent) {
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
      printf("\nERROR (nga/rre_execute): Invalid instruction!\n");
      printf("At %lld, opcode %lld\n", (long long)ip, (long long)opcode);
      exit(1);
    }
#ifndef NOCHECKS
    if (sp < 0 || sp > STACK_DEPTH) {
      printf("\nERROR (nga/rre_execute): Stack Limits Exceeded!\n");
      printf("At %lld, opcode %lld\n", (long long)ip, (long long)opcode);
      exit(1);
    }
    if (rp < 0 || rp > ADDRESSES) {
      printf("\nERROR (nga/rre_execute): Address Stack Limits Exceeded!\n");
      printf("At %lld, opcode %lld\n", (long long)ip, (long long)opcode);
      exit(1);
    }
#endif
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

void rre_evaluate(char *s, int silent) {
  if (strlen(s) == 0)  return;
//  update_rx();
  string_inject(s, TIB);
  stack_push(TIB);
  rre_execute(interpret, silent);
}


/*---------------------------------------------------------------------
  `read_token` reads a token from the specified file.  It will stop on
   a whitespace or newline. It also tries to handle backspaces, though
   the success of this depends on how your terminal is configured.
  ---------------------------------------------------------------------*/

int not_eol(int ch) {
  return (ch != 10) && (ch != 13) && (ch != 32) && (ch != EOF) && (ch != 0);
}

void read_token(FILE *file, char *token_buffer, int echo) {
  int ch = getc(file);
  int count = 0;
  if (echo != 0)
    putchar(ch);
  while (not_eol(ch)) {
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
  Display the Stack Contents
  ---------------------------------------------------------------------*/

void dump_stack() {
  CELL i;
  if (sp == 0)  return;
  printf("\nStack: ");
  for (i = 1; i <= sp; i++) {
    if (i == sp)
      printf("[ TOS: %lld ]", (long long)data[i]);
    else
      printf("%lld ", (long long)data[i]);
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
  while (!feof(fp)) {              /* Loop through the file            */
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
      if (inBlock == 1) {
        rre_evaluate(source, -1);
      }
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
  printf("Interactive Usage: %s [-h] [-i] [-f filename] [-t]\n\n", exename);
  printf("Valid Arguments:\n\n");
  printf("  -h\n");
  printf("    Display this help text\n");
  printf("  -i\n");
  printf("    Launches in interactive mode\n");
  printf("  -s\n");
  printf("    Suppress the startup banner in interactive mode. Implies -i.\n");
  printf("  -f filename\n");
  printf("    Run the contents of the specified file\n");
  printf("  -u filename\n");
  printf("    Use the image in the specified file instead of the internal one\n");
  printf("  -t\n");
  printf("    Run tests (in ``` blocks) in any loaded files\n\n");
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

int arg_is(char *argv, char *t) {
  return strcmp(argv, t) == 0;
}


/*---------------------------------------------------------------------
  Main Entry Point
  ---------------------------------------------------------------------*/

enum flags {
  FLAG_HELP, FLAG_RUN_TESTS, FLAG_INCLUDE, FLAG_INTERACTIVE, FLAG_SILENT,
};

int main(int argc, char **argv) {
  int i;
  int modes[16];
  char *files[16];
  int fsp;

  int run_tests;

  initialize();                           /* Initialize Nga & image    */

  sys_argc = argc;                        /* Point the global argc and */
  sys_argv = argv;                        /* argv to the actual ones   */

  if (argc >= 2 && argv[1][0] != '-') {
    include_file(argv[1], 0);             /* If no flags were passed,  */
    if (sp >= 1)  dump_stack();           /* load the file specified,  */
    exit(0);                              /* and exit                  */
  }

  /* Clear startup modes       */
  for (i = 0; i < 16; i++)
    modes[i] = 0;

  /* Clear startup files       */
  for (i = 0; i < 16; i++)
    files[i] = "\0";
  fsp = 0;

  run_tests = 0;

  if (argc <= 1) modes[FLAG_INTERACTIVE] = 1;

  /* Process Arguments */
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      help(argv[0]);
      exit(0);
    } else if (strcmp(argv[i], "-i") == 0) {
      modes[FLAG_INTERACTIVE] = 1;
    } else if (strcmp(argv[i], "-s") == 0) {
      modes[FLAG_INTERACTIVE] = 1;
      modes[FLAG_SILENT] = 1;
    } else if (strcmp(argv[i], "-f") == 0) {
      files[fsp] = argv[i + 1];
      fsp++;
      i++;
    } else if (strcmp(argv[i], "-u") == 0) {
      i++;
      ngaLoadImage(argv[i]);
      update_rx();
    } else if (strcmp(argv[i], "-t") == 0) {
      modes[FLAG_RUN_TESTS] = 1;
      run_tests = 1;
    }
  }

  /* Include Startup Files */
  for (i = 0; i < fsp; i++) {
    if (strcmp(files[i], "\0") != 0)
      include_file(files[i], run_tests);
  }

  /* Set Image Flags for NoEcho if started with `-s` */
  if (modes[FLAG_SILENT] == 1)
    memory[d_xt_for("NoEcho", Dictionary)] = -1;

  /* Run the Listener (if interactive mode was set) */
  if (modes[FLAG_INTERACTIVE] == 1) {
    while (1) rre_execute(0, -1);
  }
}


/*=====================================================================*/


/*---------------------------------------------------------------------
  File Handling
  ---------------------------------------------------------------------*/


/*---------------------------------------------------------------------
  I keep an array of file handles. RETRO will use the index number as
  its representation of the file.
  ---------------------------------------------------------------------*/

FILE *OpenFileHandles[MAX_OPEN_FILES];

/*---------------------------------------------------------------------
  `files_get_handle()` returns a file handle, or 0 if there are no
  available handle slots in the array.
  ---------------------------------------------------------------------*/

CELL files_get_handle() {
  CELL i;
  for(i = 1; i < MAX_OPEN_FILES; i++)
    if (OpenFileHandles[i] == 0)
      return i;
  return 0;
}


/*---------------------------------------------------------------------
  `file_open()` opens a file. This pulls from the RETRO data stack:

  - mode     (number, TOS)
  - filename (string, NOS)

  Modes are:

  | Mode | Corresponds To | Description          |
  | ---- | -------------- | -------------------- |
  |  0   | rb             | Open for reading     |
  |  1   | w              | Open for writing     |
  |  2   | a              | Open for append      |
  |  3   | rb+            | Open for read/update |

  The file name should be a NULL terminated string. This will attempt
  to open the requested file and will return a handle (index number
  into the `OpenFileHandles` array).
  ---------------------------------------------------------------------*/

void file_open() {
  CELL slot, mode, name;
  char *request;
  slot = files_get_handle();
  mode = data[sp]; sp--;
  name = data[sp]; sp--;
  request = string_extract(name);
  if (slot > 0) {
    if (mode == 0)  OpenFileHandles[slot] = fopen(request, "rb");
    if (mode == 1)  OpenFileHandles[slot] = fopen(request, "w");
    if (mode == 2)  OpenFileHandles[slot] = fopen(request, "a");
    if (mode == 3)  OpenFileHandles[slot] = fopen(request, "rb+");
  }
  if (OpenFileHandles[slot] == NULL) {
    OpenFileHandles[slot] = 0;
    slot = 0;
  }
  stack_push(slot);
}


/*---------------------------------------------------------------------
  `file_read()` reads a byte from a file. This takes a file pointer
  from the stack and pushes the character that was read to the stack.
  ---------------------------------------------------------------------*/

void file_read() {
  CELL slot = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_read): Invalid file handle\n");
    exit(1);
  }
#endif
  CELL c = fgetc(OpenFileHandles[slot]);
  stack_push(feof(OpenFileHandles[slot]) ? 0 : c);
}


/*---------------------------------------------------------------------
  `file_write()` writes a byte to a file. This takes a file pointer
  (TOS) and a byte (NOS) from the stack. It does not return any values
  on the stack.
  ---------------------------------------------------------------------*/

void file_write() {
  CELL slot, c, r;
  slot = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_write): Invalid file handle\n");
    exit(1);
  }
#endif
  c = stack_pop();
  r = fputc(c, OpenFileHandles[slot]);
}


/*---------------------------------------------------------------------
  `file_close()` closes a file. This takes a file handle from the
  stack and does not return anything on the stack.
  ---------------------------------------------------------------------*/

void file_close() {
  CELL slot = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_close): Invalid file handle\n");
    exit(1);
  }
#endif
  fclose(OpenFileHandles[slot]);
  OpenFileHandles[slot] = 0;
}


/*---------------------------------------------------------------------
  `file_get_position()` provides the current index into a file. This
  takes the file handle from the stack and returns the offset.
  ---------------------------------------------------------------------*/

void file_get_position() {
  CELL slot = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_get_position): Invalid file handle\n");
    exit(1);
  }
#endif
  stack_push((CELL) ftell(OpenFileHandles[slot]));
}


/*---------------------------------------------------------------------
  `file_set_position()` changes the current index into a file to the
  specified one. This takes a file handle (TOS) and new offset (NOS)
  from the stack.
  ---------------------------------------------------------------------*/

void file_set_position() {
  CELL slot, pos;
  slot = stack_pop();
  pos  = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_set_position): Invalid file handle\n");
    exit(1);
  }
#endif
  fseek(OpenFileHandles[slot], pos, SEEK_SET);
}


/*---------------------------------------------------------------------
  `file_get_size()` returns the size of a file, or 0 if empty. If the
  file is a directory, it returns -1. It takes a file handle from the
  stack.
  ---------------------------------------------------------------------*/

void file_get_size() {
  CELL slot, current, r, size;
  struct stat buffer;
  slot = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_get_size): Invalid file handle\n");
    exit(1);
  }
#endif
  fstat(fileno(OpenFileHandles[slot]), &buffer);
  if (!S_ISDIR(buffer.st_mode)) {
    current = ftell(OpenFileHandles[slot]);
    r = fseek(OpenFileHandles[slot], 0, SEEK_END);
    size = ftell(OpenFileHandles[slot]);
    fseek(OpenFileHandles[slot], current, SEEK_SET);
  } else {
    r = -1;
    size = 0;
  }
  stack_push((r == 0) ? size : 0);
}


/*---------------------------------------------------------------------
  `file_delete()` removes a file. This takes a file name (as a string)
  from the stack.
  ---------------------------------------------------------------------*/

void file_delete() {
  char *request;
  CELL name = data[sp]; sp--;
  request = string_extract(name);
  unlink(request);
}


/*---------------------------------------------------------------------
  `file_flush()` flushes any pending writes to disk. This takes a
  file handle from the stack.
  ---------------------------------------------------------------------*/

void file_flush() {
  CELL slot;
  slot = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_flush): Invalid file handle\n");
    exit(1);
  }
#endif
  fflush(OpenFileHandles[slot]);
}


Handler FileActions[10] = {
  file_open,
  file_close,
  file_read,
  file_write,
  file_get_position,
  file_set_position,
  file_get_size,
  file_delete,
  file_flush
};

void io_filesystem_query() {
  stack_push(0);
  stack_push(4);
}

void io_filesystem_handler() {
  FileActions[stack_pop()]();
}


/*=====================================================================*/


/*---------------------------------------------------------------------
  Floating Point
  ---------------------------------------------------------------------*/

/*---------------------------------------------------------------------
  I have a stack of floating point values ("floats") and a stack
  pointer (`fsp`).  
  ---------------------------------------------------------------------*/

double Floats[8192];
CELL fsp;

double AFloats[8192];
CELL afsp;


/*---------------------------------------------------------------------
  The first two functions push a float to the stack and pop a value off
  the stack.
  ---------------------------------------------------------------------*/

void float_push(double value) {
    fsp++;
    Floats[fsp] = value;
}

double float_pop() {
    fsp--;
    return Floats[fsp + 1];
}

void float_to_alt() {
  afsp++;
  AFloats[afsp] = float_pop();
}

void float_from_alt() {
  float_push(AFloats[afsp]);
  afsp--;
}


/*---------------------------------------------------------------------
  RETRO operates on 32-bit signed integer values. This function just
  pops a number from the data stack, casts it to a float, and pushes it
  to the float stack.
  ---------------------------------------------------------------------*/
void float_from_number() {
    float_push((double)stack_pop());
}


/*---------------------------------------------------------------------
  To get a float from a string in the image, I provide this function.
  I cheat: using `atof()` takes care of the details, so I don't have
  to.
  ---------------------------------------------------------------------*/
void float_from_string() {
    float_push(atof(string_extract(stack_pop())));
}


/*---------------------------------------------------------------------
  Converting a floating point into a string is slightly more work. Here
  I pass it off to `snprintf()` to deal with.
  ---------------------------------------------------------------------*/
void float_to_string() {
    snprintf(string_data, 8192, "%f", float_pop());
    string_inject(string_data, stack_pop());
}


/*---------------------------------------------------------------------
  Converting a floating point back into a standard number requires a
  little care due to the signed nature. This makes adjustments for the
  max & min value, and then casts (rounding) the float back to a normal
  number.
  ---------------------------------------------------------------------*/

void float_to_number() {
    double a = float_pop();
    if (a > 2147483647)
      a = 2147483647;
    if (a < -2147483648)
      a = -2147483648;
    stack_push((CELL)round(a));
}


/*---------------------------------------------------------------------
  Now I get to define a bunch of functions that operate on floats.
  These provide the basic math, and wrappers around functionality in
  libm.
  ---------------------------------------------------------------------*/

void float_add() {
    double a = float_pop();
    double b = float_pop();
    float_push(a+b);
}

void float_sub() {
    double a = float_pop();
    double b = float_pop();
    float_push(b-a);
}

void float_mul() {
    double a = float_pop();
    double b = float_pop();
    float_push(a*b);
}

void float_div() {
  double a = float_pop();
  double b = float_pop();
  float_push(b/a);
}

void float_floor() {
    float_push(floor(float_pop()));
}

void float_ceil() {
    float_push(ceil(float_pop()));
}

void float_eq() {
    double a = float_pop();
    double b = float_pop();
    if (a == b)
        stack_push(-1);
    else
        stack_push(0);
}

void float_neq() {
    double a = float_pop();
    double b = float_pop();
    if (a != b)
        stack_push(-1);
    else
        stack_push(0);
}

void float_lt() {
    double a = float_pop();
    double b = float_pop();
    if (b < a)
        stack_push(-1);
    else
        stack_push(0);
}

void float_gt() {
    double a = float_pop();
    double b = float_pop();
    if (b > a)
        stack_push(-1);
    else
        stack_push(0);
}

void float_depth() {
    stack_push(fsp);
}

void float_adepth() {
    stack_push(afsp);
}

void float_dup() {
    double a = float_pop();
    float_push(a);
    float_push(a);
}

void float_drop() {
    float_pop();
}

void float_swap() {
    double a = float_pop();
    double b = float_pop();
    float_push(a);
    float_push(b);
}

void float_log() {
    double a = float_pop();
    double b = float_pop();
    float_push(log(b) / log(a));
}

void float_sqrt() {
  float_push(sqrt(float_pop()));
}

void float_pow() {
    double a = float_pop();
    double b = float_pop();
    float_push(pow(b, a));
}

void float_sin() {
  float_push(sin(float_pop()));
}

void float_cos() {
  float_push(cos(float_pop()));
}

void float_tan() {
  float_push(tan(float_pop()));
}

void float_asin() {
  float_push(asin(float_pop()));
}

void float_acos() {
  float_push(acos(float_pop()));
}

void float_atan() {
  float_push(atan(float_pop()));
}


/*---------------------------------------------------------------------
  With this finally done, I implement the FPU instructions.
  ---------------------------------------------------------------------*/
Handler FloatHandlers[] = {
  float_from_number,  float_from_string,  float_to_number,  float_to_string,
  float_add,          float_sub,          float_mul,        float_div,
  float_floor,        float_ceil,         float_sqrt,       float_eq,
  float_neq,          float_lt,           float_gt,         float_depth,
  float_dup,          float_drop,         float_swap,       float_log,
  float_pow,          float_sin,          float_tan,        float_cos,
  float_asin,         float_acos,         float_atan,       float_to_alt,
  float_from_alt,     float_adepth,
};

void io_floatingpoint_query() {
  stack_push(1);
  stack_push(2);
}

void io_floatingpoint_handler() {
  FloatHandlers[stack_pop()]();
}


/*=====================================================================*/


/*---------------------------------------------------------------------
  `unix_open_pipe()` is like `file_open()`, but for pipes. This pulls
  from the data stack:

  - mode       (number, TOS)
  - executable (string, NOS)

  Modes are:

  | Mode | Corresponds To | Description          |
  | ---- | -------------- | -------------------- |
  |  0   | r              | Open for reading     |
  |  1   | w              | Open for writing     |
  |  3   | r+             | Open for read/update |

  The file name should be a NULL terminated string. This will attempt
  to open the requested file and will return a handle (index number
  into the `OpenFileHandles` array).

  Once opened, you can use the standard file words to read/write to the
  process.
  ---------------------------------------------------------------------*/

void unix_open_pipe() {
  CELL slot, mode, name;
  char *request;
  slot = files_get_handle();
  mode = stack_pop();
  name = stack_pop();
  request = string_extract(name);
  if (slot > 0) {
    if (mode == 0)  OpenFileHandles[slot] = popen(request, "r");
    if (mode == 1)  OpenFileHandles[slot] = popen(request, "w");
    if (mode == 3)  OpenFileHandles[slot] = popen(request, "r+");
  }
  if (OpenFileHandles[slot] == NULL) {
    OpenFileHandles[slot] = 0;
    slot = 0;
  }
  stack_push(slot);
}

void unix_close_pipe() {
  pclose(OpenFileHandles[data[sp]]);
  OpenFileHandles[data[sp]] = 0;
  sp--;
}

void unix_system() {
  system(string_extract(stack_pop()));
}

void unix_fork() {
  stack_push(fork());
}

/*---------------------------------------------------------------------
  UNIX provides `execl` to execute a file, with various forms for
  arguments provided.

  RRE wraps this in several functions, one for each number of passed
  arguments. See the Glossary for details on what each takes from the
  stack. Each of these will return the error code if the execution
  fails.
  ---------------------------------------------------------------------*/

void unix_exec0() {
  char path[1025];
  bsd_strlcpy(path, string_extract(stack_pop()), 1024);
  execl(path, path, (char *)0);
  stack_push(errno);
}

void unix_exec1() {
  char path[1025];
  char arg0[1025];
  bsd_strlcpy(arg0, string_extract(stack_pop()), 1024);
  bsd_strlcpy(path, string_extract(stack_pop()), 1024);
  execl(path, path, arg0, (char *)0);
  stack_push(errno);
}

void unix_exec2() {
  char path[1025];
  char arg0[1025], arg1[1025];
  bsd_strlcpy(arg1, string_extract(stack_pop()), 1024);
  bsd_strlcpy(arg0, string_extract(stack_pop()), 1024);
  bsd_strlcpy(path, string_extract(stack_pop()), 1024);
  execl(path, path, arg0, arg1, (char *)0);
  stack_push(errno);
}

void unix_exec3() {
  char path[1025];
  char arg0[1025], arg1[1025], arg2[1025];
  bsd_strlcpy(arg2, string_extract(stack_pop()), 1024);
  bsd_strlcpy(arg1, string_extract(stack_pop()), 1024);
  bsd_strlcpy(arg0, string_extract(stack_pop()), 1024);
  bsd_strlcpy(path, string_extract(stack_pop()), 1024);
  execl(path, path, arg0, arg1, arg2, (char *)0);
  stack_push(errno);
}

void unix_exit() {
  exit(stack_pop());
}

void unix_getpid() {
  stack_push(getpid());
}

void unix_wait() {
  int a;
  stack_push(wait(&a));
}

void unix_kill() {
  CELL a;
  a = stack_pop();
  kill(stack_pop(), a);
}

void unix_write() {
  CELL a, b, c;
  c = stack_pop();
  b = stack_pop();
  a = stack_pop();
  write(fileno(OpenFileHandles[c]), string_extract(a), b);
}

void unix_chdir() {
  chdir(string_extract(stack_pop()));
}

void unix_getenv() {
  CELL a, b;
  a = stack_pop();
  b = stack_pop();
  string_inject(getenv(string_extract(b)), a);
}

void unix_putenv() {
  putenv(string_extract(stack_pop()));
}

void unix_sleep() {
  sleep(stack_pop());
}


/*---------------------------------------------------------------------
  Faster verisons of `n:put` and `s:put`
  ---------------------------------------------------------------------*/

void unix_io_putn() {
  printf("%ld", (long)stack_pop());
}

void unix_io_puts() {
  printf("%s", string_extract(stack_pop()));
}


/*---------------------------------------------------------------------
  Time and Date Functions
  ---------------------------------------------------------------------*/
void unix_time() {
  stack_push((CELL)time(NULL));
}

void unix_time_day() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_mday);
}

void unix_time_month() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_mon + 1);
}

void unix_time_year() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_year + 1900);
}

void unix_time_hour() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_hour);
}

void unix_time_minute() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_min);
}

void unix_time_second() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_sec);
}

void unix_time_day_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_mday);
}

void unix_time_month_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_mon + 1);
}

void unix_time_year_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_year + 1900);
}

void unix_time_hour_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_hour);
}

void unix_time_minute_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_min);
}

void unix_time_second_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_sec);
}

Handler UnixActions[] = {
  unix_system,    unix_fork,       unix_exec0,   unix_exec1,   unix_exec2,
  unix_exec3,     unix_exit,       unix_getpid,  unix_wait,    unix_kill,
  unix_open_pipe, unix_close_pipe, unix_write,   unix_chdir,   unix_getenv,
  unix_putenv,    unix_sleep,      unix_io_putn, unix_io_puts, unix_time,
  unix_time_day,      unix_time_month,      unix_time_year,
  unix_time_hour,     unix_time_minute,     unix_time_second,
  unix_time_day_utc,  unix_time_month_utc,  unix_time_year_utc,
  unix_time_hour_utc, unix_time_minute_utc, unix_time_second_utc
};

void io_unix_query() {
  stack_push(1);
  stack_push(8);
}

void io_unix_handler() {
  UnixActions[stack_pop()]();
}


/*=====================================================================*/

/*---------------------------------------------------------------------
  BSD Sockets
  ---------------------------------------------------------------------*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

int SocketID[16];
struct sockaddr_in Sockets[16];

struct addrinfo hints, *res;

void socket_getaddrinfo() {
  char host[1025], port[6];
  bsd_strlcpy(port, string_extract(stack_pop()), 5);
  bsd_strlcpy(host, string_extract(stack_pop()), 1024);
  getaddrinfo(host, port, &hints, &res);
}

void socket_get_host() {
  struct hostent *hp;
  struct in_addr **addr_list;

  hp = gethostbyname(string_extract(stack_pop()));
  if (hp == NULL) {
    memory[stack_pop()] = 0;
    return;
  }

  addr_list = (struct in_addr **)hp->h_addr_list;
  string_inject(inet_ntoa(*addr_list[0]), stack_pop());
}

void socket_create() {
  int i;
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  for (i = 0; i < 16; i++) {
    if (SocketID[i] == 0 && sock != 0) {
      SocketID[i] = sock;
      stack_push((CELL)i);
      sock = 0;
    }
  }
}

void socket_bind() {
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int sock = stack_pop();
  int port = stack_pop();

  getaddrinfo(NULL, string_extract(port), &hints, &res);
  stack_push((CELL) bind(SocketID[sock], res->ai_addr, res->ai_addrlen));
  stack_push(errno);
}

void socket_listen() {
  int sock = stack_pop();
  int backlog = stack_pop();
  stack_push(listen(SocketID[sock], backlog));
  stack_push(errno);
}

void socket_accept() {
  int i;
  int sock = stack_pop();
  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;
  int new_fd = accept(SocketID[sock], (struct sockaddr *)&their_addr, &addr_size);

  for (i = 0; i < 16; i++) {
    if (SocketID[i] == 0 && new_fd != 0) {
      SocketID[i] = new_fd;
      stack_push((CELL)i);
      new_fd = 0;
    }
  }
  stack_push(errno);
}

void socket_connect() {
  stack_push((CELL)connect(SocketID[stack_pop()], res->ai_addr, res->ai_addrlen));
  stack_push(errno);
}

void socket_send() {
  int sock = stack_pop();
  char *buf = string_extract(stack_pop());
  stack_push(send(SocketID[sock], buf, strlen(buf), 0));
  stack_push(errno);
}

void socket_sendto() {
}

void socket_recv() {
  char buf[8193];
  int sock = stack_pop();
  int limit = stack_pop();
  int dest = stack_pop();
  int len = recv(SocketID[sock], buf, limit, 0);
  if (len > 0)  buf[len] = '\0';
  if (len > 0)  string_inject(buf, dest);
  stack_push(len);
  stack_push(errno);
}

void socket_recvfrom() {
}

void socket_close() {
  int sock = stack_pop();
  close(SocketID[sock]);
  SocketID[sock] = 0;
}

Handler SocketActions[] = {
  socket_get_host,
  socket_create, socket_bind,    socket_listen,
  socket_accept, socket_connect, socket_send,
  socket_sendto, socket_recv,    socket_recvfrom,
  socket_close, socket_getaddrinfo
};

void io_socket() {
  SocketActions[stack_pop()]();
}

void query_socket() {
  stack_push(0);
  stack_push(7);
}

/*=====================================================================*/


/*---------------------------------------------------------------------
  Interfacing With The Image
  ---------------------------------------------------------------------*/

/*---------------------------------------------------------------------
  Stack push/pop is easy. I could avoid these, but it aids in keeping
  the code readable, so it's worth the slight overhead.
  ---------------------------------------------------------------------*/

CELL stack_pop() {
  sp--;
#ifndef NOCHECKS
  if (sp < 0) {
    printf("\nERROR (nga/stack_pop): Data stack underflow.\n");
    exit(1);
  }
#endif
  return data[sp + 1];
}

void stack_push(CELL value) {
  sp++;
#ifndef NOCHECKS
  if (sp >= STACK_DEPTH) {
    printf("\nERROR (nga/stack_push): Data stack overflow.\n");
    exit(1);
  }
#endif
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
  Compiler = d_xt_for("Compiler", Compiler);
}


/*=====================================================================*/


/*---------------------------------------------------------------------
  Instruction Processor
  ---------------------------------------------------------------------*/

enum vm_opcode {
  VM_NOP,  VM_LIT,    VM_DUP,   VM_DROP,    VM_SWAP,   VM_PUSH,  VM_POP,
  VM_JUMP, VM_CALL,   VM_CCALL, VM_RETURN,  VM_EQ,     VM_NEQ,   VM_LT,
  VM_GT,   VM_FETCH,  VM_STORE, VM_ADD,     VM_SUB,    VM_MUL,   VM_DIVMOD,
  VM_AND,  VM_OR,     VM_XOR,   VM_SHIFT,   VM_ZRET,   VM_END,   VM_IE,
  VM_IQ,   VM_II
};
#define NUM_OPS VM_II + 1

CELL ngaLoadImage(char *imageFile) {
  FILE *fp;
  CELL imageSize;
  long fileLen;
  CELL i;
  if ((fp = fopen(imageFile, "rb")) != NULL) {
    /* Determine length (in cells) */
    fseek(fp, 0, SEEK_END);
    fileLen = ftell(fp) / sizeof(CELL);
    if (fileLen > IMAGE_SIZE) {
      fclose(fp);
      printf("\nERROR (nga/ngaLoadImage): Image is larger than alloted space!\n");
      exit(1);
    }
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
#ifndef NOCHECKS
  if (TOS >= IMAGE_SIZE || TOS < -6) {
    ip = IMAGE_SIZE;
    printf("\nERROR (nga/inst_fetch): Fetch beyond valid memory range\n");
    exit(1);
  } else {
#endif
    switch (TOS) {
      case -1: TOS = sp - 1; break;
      case -2: TOS = rp; break;
      case -3: TOS = IMAGE_SIZE; break;
      case -4: TOS = CELL_MIN; break;
      case -5: TOS = CELL_MAX; break;
      default: TOS = memory[TOS]; break;
    }
#ifndef NOCHECKS
  }
#endif
}

void inst_store() {
#ifndef NOCHECKS
  if (TOS <= IMAGE_SIZE && TOS >= 0) {
#endif
    memory[TOS] = NOS;
    inst_drop();
    inst_drop();
#ifndef NOCHECKS
  } else {
    ip = IMAGE_SIZE;
    printf("\nERROR (nga/inst_store): Store beyond valid memory range\n");
    exit(1);
  }
#endif
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
#ifndef NOCHECKS
  if (a == 0) {
    printf("\nERROR (nga/inst_divmod): Division by zero\n");
    exit(1);
  }
#endif
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


/*=====================================================================*/

/*---------------------------------------------------------------------
  I use the strl* functions for extracting and copying strings. These
  are present in the BSD systems I use mostly, but are missing on
  Linux, AIX, HP-UX, and maybe others. For this reason, I include
  copies here.

  Copyright (c) 1998, 2015 Todd C. Miller <Todd.Miller@courtesan.com>
 
  Permission to use, copy, modify, and distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.
 
  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
  --------------------------------------------------------------------*/

size_t bsd_strlcat(char *dst, const char *src, size_t dsize) {
  const char *odst = dst;
  const char *osrc = src;
  size_t n = dsize;
  size_t dlen;

  /* Find the end of dst and adjust bytes left but don't go past end. */
  while (n-- != 0 && *dst != '\0')
    dst++;
  dlen = dst - odst;
  n = dsize - dlen;

  if (n-- == 0)
    return(dlen + strlen(src));
  while (*src != '\0') {
    if (n != 0) {
      *dst++ = *src;
      n--;
    }
    src++;
  }
  *dst = '\0';
  return(dlen + (src - osrc));	/* count does not include NUL */
}

size_t bsd_strlcpy(char *dst, const char *src, size_t dsize) {
  const char *osrc = src;
  size_t nleft = dsize;

  /* Copy as many bytes as will fit. */
  if (nleft != 0) {
    while (--nleft != 0) {
      if ((*dst++ = *src++) == '\0')
        break;
    }
  }

  /* Not enough room in dst, add NUL and traverse rest of src. */
  if (nleft == 0) {
    if (dsize != 0)
      *dst = '\0';		/* NUL-terminate dst */
    while (*src++)
      ;
  }
  return(src - osrc - 1);	/* count does not include NUL */
}
