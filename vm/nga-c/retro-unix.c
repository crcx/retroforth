/*---------------------------------------------------------------------
  RETRO is a personal, minimalistic forth with a pragmatic focus

  This implements Nga, the virtual machine at the heart of RETRO. It
  includes a number of I/O interfaces, extensive commentary, and has
  been refined by over a decade of use and development.

  Copyright (c) 2008 - 2021, Charles Childers

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

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

/*---------------------------------------------------------------------
  Configuration
  ---------------------------------------------------------------------*/

#include "config.h"
#include "prototypes.h"

/*---------------------------------------------------------------------
  Image, Stack, and VM variables
  ---------------------------------------------------------------------*/

CELL memory[IMAGE_SIZE + 1];      /* The memory for the image          */

#define TOS  cpu.data[cpu.sp]     /* Shortcut for top item on stack    */
#define NOS  cpu.data[cpu.sp-1]   /* Shortcut for second item on stack */
#define TORS cpu.address[cpu.rp]  /* Shortcut for top item on address stack */

struct NgaCore {
  CELL sp, rp, ip;                /* Stack & instruction pointers      */
  CELL data[STACK_DEPTH];         /* The data stack                    */
  CELL address[ADDRESSES];        /* The address stack                 */
} cpu;

/*---------------------------------------------------------------------
  Markers for code & test blocks
  ---------------------------------------------------------------------*/

char code_start[33], code_end[33], test_start[33], test_end[33];



/*---------------------------------------------------------------------
  Populate The I/O Device Tables
  ---------------------------------------------------------------------*/

typedef void (*Handler)(void);

Handler IO_deviceHandlers[] = {
  io_output_handler,
  io_keyboard_handler,
  io_floatingpoint_handler,
  io_filesystem_handler,
  io_clock_handler,
  io_unix_handler,
  io_scripting_handler,
  io_random,
  io_image,
};

Handler IO_queryHandlers[] = {
  io_output_query,
  io_keyboard_query,
  io_floatingpoint_query,
  io_filesystem_query,
  io_clock_query,
  io_unix_query,
  io_scripting_query,
  io_random_query,
  io_image_query,
};


/*---------------------------------------------------------------------
  Variables Related To Image Introspection
  ---------------------------------------------------------------------*/

CELL Compiler;
CELL Dictionary;
CELL NotFound;
CELL interpret;


/*---------------------------------------------------------------------
  Global Variables
  ---------------------------------------------------------------------*/

char string_data[8192];
char **sys_argv;
int sys_argc;
int silence_input;
char scripting_sources[64][8192];
int current_source;
int perform_abort;

/*---------------------------------------------------------------------
  Embed The Image and Devices
  ---------------------------------------------------------------------*/

#include "retro-image.c"
#include "bsd-strl.c"
#include "dev-floatingpoint.c"
#include "dev-image.c"
#include "dev-files.c"
#include "dev-unix.c"
#include "dev-clock.c"
#include "dev-rng.c"


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

CELL currentLine;
CELL ignoreToEOL;
CELL ignoreToEOF;

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

/* addeded in scripting i/o device, revision 1 */
void scripting_source() {
  stack_push(string_inject(scripting_sources[current_source], stack_pop()));
}

void scripting_line() {
  stack_push(currentLine + 1);
}

void scripting_ignore_to_eol() {
  ignoreToEOL = -1;
}

void scripting_ignore_to_eof() {
  ignoreToEOF = -1;
}

void scripting_abort() {
  scripting_ignore_to_eol();
  scripting_ignore_to_eof();
  perform_abort = -1;
}

void carry_out_abort() {
  cpu.ip = IMAGE_SIZE + 1;
  cpu.rp = 0;
  cpu.sp = 0;
  fsp = 0;
  afsp = 0;

  if (current_source > 0) {
    scripting_abort();
    return;
  }

  perform_abort = 0;
  current_source = 0;
}

Handler ScriptingActions[] = {
  scripting_arg_count,
  scripting_arg,
  scripting_include,
  scripting_name,
  scripting_source,
  scripting_line,
  scripting_ignore_to_eol,
  scripting_ignore_to_eof,
  scripting_abort
};

void io_scripting_query() {
  stack_push(2);
  stack_push(9);
}

void io_scripting_handler() {
  ScriptingActions[stack_pop()]();
}


/*=====================================================================*/

/*---------------------------------------------------------------------
  With these out of the way, I implement `execute`, which takes an
  address and runs the code at it. This has a couple of interesting
  bits.

  This will also exit if the address stack depth is zero (meaning that
  the word being run, and it's dependencies) are finished.
  ---------------------------------------------------------------------*/

void invalid_opcode(CELL opcode) {
  CELL a, i;
  printf("\nERROR (nga/execute): Invalid instruction!\n");
  printf("At %lld, opcode %lld\n", (long long)cpu.ip, (long long)opcode);
  printf("Instructions: ");
  a = opcode;
  for (i = 0; i < 4; i++) {
    printf("%lldd ", (long long)a & 0xFF);
    a = a >> 8;
  }
  printf("\n");
  exit(1);
}

void execute(CELL cell, int silent) {
  CELL token;
  CELL opcode;
  silence_input = silent;
  if (cpu.rp == 0)
    cpu.rp = 1;
  cpu.ip = cell;
  token = TIB;
  while (cpu.ip < IMAGE_SIZE) {
    if (perform_abort == 0) {
      if (cpu.ip == NotFound) {
        printf("\nERROR: Word Not Found: ");
        printf("`%s`\n\n", string_extract(token));
      }
      if (cpu.ip == interpret) {
        token = TOS;
      }
      opcode = memory[cpu.ip];
      if (validate_opcode_bundle(opcode) != 0) {
        process_opcode_bundle(opcode);
      } else {
        invalid_opcode(opcode);
      }
#ifndef NOCHECKS
      if (cpu.sp < 0 || cpu.sp > STACK_DEPTH) {
        printf("\nERROR (nga/execute): Stack Limits Exceeded!\n");
        printf("At %lld, opcode %lld. sp = %lld\n", (long long)cpu.ip, (long long)opcode, (long long)cpu.sp);
        exit(1);
      }
      if (cpu.rp < 0 || cpu.rp > ADDRESSES) {
        printf("\nERROR (nga/execute): Address Stack Limits Exceeded!\n");
        printf("At %lld, opcode %lld. rp = %lld\n", (long long)cpu.ip, (long long)opcode, (long long)cpu.rp);
        exit(1);
      }
#endif
      cpu.ip++;
      if (cpu.rp == 0)
        cpu.ip = IMAGE_SIZE;
    } else {
      carry_out_abort();
    }
  }
}


/*---------------------------------------------------------------------
  RETRO's `interpret` word expects a token on the stack. This next
  function copies a token to the `TIB` (text input buffer) and then
  calls `interpret` to process it.
  ---------------------------------------------------------------------*/

void evaluate(char *s, int silent) {
  if (strlen(s) == 0)  return;
  string_inject(s, TIB);
  stack_push(TIB);
  execute(interpret, silent);
}


/*---------------------------------------------------------------------
  `read_token` reads a token from the specified file.  It will stop on
   a whitespace or newline. It also tries to handle backspaces, though
   the success of this depends on how your terminal is configured.
  ---------------------------------------------------------------------*/

int not_eol(int c) {
  return (c != 10) && (c != 13) && (c != 32) && (c != EOF) && (c != 0);
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
  if (cpu.sp == 0)  return;
  printf("\nStack: ");
  for (i = 1; i <= cpu.sp; i++) {
    if (i == cpu.sp)
      printf("[ TOS: %lld ]", (long long)cpu.data[i]);
    else
      printf("%lld ", (long long)cpu.data[i]);
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

/* Check to see if a line is a fence boundary.
   This will check code blocks in all cases, and test blocks
   if tests_enabled is set to a non-zero value. */

int fence_boundary(char *buffer, int tests_enabled) {
  int flag = 1;
  if (strcmp(buffer, code_start) == 0) { flag = -1; }
  if (strcmp(buffer, code_end) == 0)   { flag = -1; }
  if (tests_enabled == 0) { return flag; }
  if (strcmp(buffer, test_start) == 0) { flag = -1; }
  if (strcmp(buffer, test_end) == 0)   { flag = -1; }
  return flag;
}


/*---------------------------------------------------------------------
  And now for the actual `include_file()` function.
  ---------------------------------------------------------------------*/

void read_line(FILE *file, char *token_buffer) {
  int ch = getc(file);
  int count = 0;
  while ((ch != 10) && (ch != 13) && (ch != EOF) && (ch != 0)) {
    token_buffer[count++] = ch;
    ch = getc(file);
  }
  token_buffer[count] = '\0';
}

int count_tokens(char *line) {
  char ch = line[0];
  int count = 1;
  while (*line++) {
    ch = line[0];
    if (isspace(ch))
      count++;
  }
  return count;
}

void include_file(char *fname, int run_tests) {
  int inBlock = 0;                 /* Tracks status of in/out of block */
  char source[64 * 1024];          /* Token buffer [about 64K]         */
  char line[64 * 1024];            /* Line buffer [about 64K]          */
  char fence[33];                  /* Used with `fence_boundary()`     */

  CELL ReturnStack[ADDRESSES];
  CELL arp, aip;
  
  long offset = 0;
  CELL at = 0;
  int tokens = 0;
  FILE *fp;                        /* Open the file. If not found,     */
  fp = fopen(fname, "r");          /* exit.                            */
  if (fp == NULL)
    return;

  arp = cpu.rp;
  aip = cpu.ip;
  for(cpu.rp = 0; cpu.rp <= arp; cpu.rp++)
    ReturnStack[cpu.rp] = cpu.address[cpu.rp];
  cpu.rp = 0;
  
  current_source++;
   bsd_strlcpy(scripting_sources[current_source], fname, 8192);

  ignoreToEOF = 0;

  while (!feof(fp) && (ignoreToEOF == 0)) { /* Loop through the file   */

    ignoreToEOL = 0;

    offset = ftell(fp);
    read_line(fp, line);
    fseek(fp, offset, SEEK_SET);

    tokens = count_tokens(line);

    while (tokens > 0 && ignoreToEOL == 0) {
      tokens--;
      read_token(fp, source, 0);
      bsd_strlcpy(fence, source, 32); /* Copy the first three characters  */
      if (fence_boundary(fence, run_tests) == -1) {
        if (inBlock == 0)
          inBlock = 1;
        else
          inBlock = 0;
      } else {
        if (inBlock == 1) {
          currentLine = at;
          evaluate(source, -1);
          currentLine = at;
        }
      }
    }
    if (ignoreToEOL == -1)
      read_line(fp, line);
    at++;
  }

  current_source--;
  ignoreToEOF = 0;
  fclose(fp);
  if (perform_abort == -1) {
    carry_out_abort();
  }
  for(cpu.rp = 0; cpu.rp <= arp; cpu.rp++)
    cpu.address[cpu.rp] = ReturnStack[cpu.rp];
  cpu.rp = arp;
  cpu.ip = aip;
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
  printf("  -r filename\n");
  printf("    Use the image in the specified file instead of the internal one and run the code in it\n");
  printf("  -t\n");
  printf("    Run tests (in ``` blocks) in any loaded files\n\n");
}


/*---------------------------------------------------------------------
  `initialize()` sets up Nga and loads the image (from the array in
  `image.c`) to memory.
  ---------------------------------------------------------------------*/

void initialize() {
  CELL i;
  prepare_vm();
  for (i = 0; i < ngaImageCells; i++)
    memory[i] = ngaImage[i];
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
  FLAG_RUN,
};

int main(int argc, char **argv) {
  int i;
  int modes[16];
  char *files[16];
  int fsp;

  int run_tests;

  initialize();                           /* Initialize Nga & image    */

  strcpy(code_start, "~~~");
  strcpy(code_end,   "~~~");
  strcpy(test_start, "```");
  strcpy(test_end,   "```");

  /* Setup variables related to the scripting device */
  currentLine = 0;                        /* Current Line # for script */
  current_source = 0;                     /* Current file being run    */
  perform_abort = 0;                      /* Carry out abort procedure */
  sys_argc = argc;                        /* Point the global argc and */
  sys_argv = argv;                        /* argv to the actual ones   */
  bsd_strlcpy(scripting_sources[0], "/dev/stdin", 8192);
  ignoreToEOL = 0;
  ignoreToEOF = 0;

  if (argc >= 2 && argv[1][0] != '-') {
    update_rx();
    include_file(argv[1], 0);             /* If no flags were passed,  */
    if (cpu.sp >= 1)  dump_stack();       /* load the file specified,  */
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
      load_image(argv[i]);
    } else if (strcmp(argv[i], "-r") == 0) {
      i++;
      load_image(argv[i]);
      modes[FLAG_RUN] = 1;
    } else if (strcmp(argv[i], "-t") == 0) {
      modes[FLAG_RUN_TESTS] = 1;
      run_tests = 1;
    } else  if (arg_is(argv[i], "--code-start") || arg_is(argv[i], "-cs")) {
      i++;
      strcpy(code_start, argv[i]);
    } else if (arg_is(argv[i], "--code-end") || arg_is(argv[i], "-ce")) {
      i++;
      strcpy(code_end, argv[i]);
    } else if (arg_is(argv[i], "--test-start") || arg_is(argv[i], "-ts")) {
      i++;
      strcpy(test_start, argv[i]);
    } else if (arg_is(argv[i], "--test-end") || arg_is(argv[i], "-te")) {
      i++;
      strcpy(test_end, argv[i]);
    }
  }

  update_rx();

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
    execute(0, -1);
  }

  if (modes[FLAG_RUN] == 1) {
    execute(0, -1);
  }
}


/*=====================================================================*/


/*=====================================================================*/


/*---------------------------------------------------------------------
  Interfacing With The Image
  ---------------------------------------------------------------------*/

/*---------------------------------------------------------------------
  Stack push/pop is easy. I could avoid these, but it aids in keeping
  the code readable, so it's worth the slight overhead.
  ---------------------------------------------------------------------*/

CELL stack_pop() {
  cpu.sp--;
#ifndef NOCHECKS
  if (cpu.sp < 0) {
    printf("\nERROR (nga/stack_pop): Data stack underflow.\n");
    exit(1);
  }
#endif
  return cpu.data[cpu.sp + 1];
}

void stack_push(CELL value) {
  cpu.sp++;
#ifndef NOCHECKS
  if (cpu.sp >= STACK_DEPTH) {
    printf("\nERROR (nga/stack_push): Data stack overflow.\n");
    exit(1);
  }
#endif
  cpu.data[cpu.sp] = value;
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

#include "nga.c"
