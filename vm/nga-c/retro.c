/**************************************************************
                _              __            _   _
       _ __ ___| |_ _ __ ___  / _| ___  _ __| |_| |__
      | '__/ _ \ __| '__/ _ \| |_ / _ \| '__| __| '_ \
      | | |  __/ |_| | | (_) |  _| (_) | |  | |_| | | |
      |_|  \___|\__|_|  \___/|_|  \___/|_|   \__|_| |_|
                                                for nga

      (c) Charles Childers, Luke Parrish, Marc Simpsonn,
          Jay Skeer, Kenneth Keating

**************************************************************/

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "devices.h"

#define ACTIVE vm->cpu[vm->active]
#define TIB vm->memory[7]

#define MAX_DEVICES      32
#define MAX_OPEN_FILES   32

#include "image.c"

typedef struct NgaState NgaState;

typedef void (*Handler)(NgaState *);

struct NgaCore {
  CELL sp, rp, ip;            /* Stack & instruction pointers */
  CELL active;                /* Is core active?              */
  CELL u;                     /* Should next operation be     */
                              /* unsigned?                    */
  CELL data[STACK_DEPTH];     /* The data stack               */
  CELL address[ADDRESSES];    /* The address stack            */

#ifdef ENABLE_MULTICORE
  CELL registers[24];         /* Internal Registers           */
#endif
};

struct NgaState {
  /* System Memory */
  CELL memory[IMAGE_SIZE + 1];

  /* CPU Cores */
  struct NgaCore cpu[CORES];
  int active;

  /* I/O Devices */
  int devices;
  Handler IO_deviceHandlers[MAX_DEVICES];
  Handler IO_queryHandlers[MAX_DEVICES];

  CELL Dictionary, interpret;    /* Interfacing     */
  char string_data[8192];

#ifdef ENABLE_FLOATS
  double Floats[256], AFloats[256];        /* Floating Point */
  CELL fsp, afsp;
#endif

#ifdef ENABLE_BLOCKS
  char BlockFile[1025];
#endif

#ifdef ENABLE_ERROR
  CELL ErrorHandlers[64];
#endif

  /* Scripting */
  char **sys_argv;
  int sys_argc;
  char scripting_sources[64][8192];
  char line[4096];
  int current_source;
  int perform_abort;
  int interactive;

  CELL currentLine;
  CELL ignoreToEOL, ignoreToEOF;

  /* Configuration of code & test fences for Unu */
  char code_start[256], code_end[256];
  char test_start[256], test_end[256];
  int codeBlocks;

  FILE *OpenFileHandles[MAX_OPEN_FILES];
};



#define V void

#define IO(name) void io_name(NgaState *); void query_name(NgaState *);


/* Function Prototypes ----------------------------------------------- */
V handle_error(NgaState *, CELL);

CELL stack_pop(NgaState *);
V stack_push(NgaState *, CELL);
CELL string_inject(NgaState *, char *, CELL);
char *string_extract(NgaState *, CELL);
V update_rx(NgaState *);
V include_file(NgaState *, char *, int);

V register_device(NgaState *, V *, V *);

IO(output)
IO(keyboard)
IO(filesystem)
IO(scripting)
IO(rng)
IO(unsigned)

#ifdef ENABLE_UNIX
IO(unix)
#endif

#ifdef ENABLE_FLOATS
IO(floatingpoint)
#endif

#ifdef ENABLE_SOCKETS
IO(socket)
#endif

#ifdef ENABLE_MALLOC
#ifdef BIT64
IO(malloc)
#endif
#endif

#ifdef ENABLE_BLOCKS
IO(blocks)
#endif

IO(image)

V load_embedded_image(NgaState *);
CELL load_image(NgaState *, char *);
V prepare_vm(NgaState *);
V process_opcode_bundle(NgaState *, CELL);
V validate_opcode_bundle(NgaState *, CELL);

#ifdef NEEDS_STRL
size_t strlcat(char *dst, const char *src, size_t dsize);
size_t strlcpy(char *dst, const char *src, size_t dsize);
#endif

V inst_no(NgaState *);  V inst_li(NgaState *);
V inst_du(NgaState *);  V inst_dr(NgaState *);
V inst_sw(NgaState *);  V inst_pu(NgaState *);
V inst_po(NgaState *);  V inst_ju(NgaState *);
V inst_ca(NgaState *);  V inst_cc(NgaState *);
V inst_re(NgaState *);  V inst_eq(NgaState *);
V inst_ne(NgaState *);  V inst_lt(NgaState *);
V inst_gt(NgaState *);  V inst_fe(NgaState *);
V inst_st(NgaState *);  V inst_ad(NgaState *);
V inst_su(NgaState *);  V inst_mu(NgaState *);
V inst_di(NgaState *);  V inst_an(NgaState *);
V inst_or(NgaState *);  V inst_xo(NgaState *);
V inst_sh(NgaState *);  V inst_zr(NgaState *);
V inst_ha(NgaState *);  V inst_ie(NgaState *);
V inst_iq(NgaState *);  V inst_ii(NgaState *);


/* Image, Stack, and VM variables ------------------------------------ */
#define TOS  ACTIVE.data[ACTIVE.sp]
#define NOS  ACTIVE.data[ACTIVE.sp-1]
#define TORS ACTIVE.address[ACTIVE.rp]

/* Global Variables -------------------------------------------------- */

int verbose;

V guard(NgaState *vm, int n, int m, int diff) {
  if (ACTIVE.sp < n) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[1] != 0) {
      handle_error(vm, 1);
    }
#else
    printf("E: Data Stack Underflow");
    ACTIVE.sp = 0;
    return;
#endif
  }
  if (((ACTIVE.sp + m) - n) > (STACK_DEPTH - 1)) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[2] != 0) {
      handle_error(vm, 2);
    }
#else
    printf("E: Data Stack Overflow");
    ACTIVE.sp = 0;
    return;
#endif
  }
  if (diff) {
    if (ACTIVE.rp + diff < 0) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[3] != 0) {
      handle_error(vm, 3);
    }
#else
      return;
#endif
    }
    if (ACTIVE.rp + diff > (ADDRESSES - 1)) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[1] != 4) {
      handle_error(vm, 4);
    }
#else
      return;
#endif
    }
  }
}

#ifdef ENABLE_MALLOC
#ifdef BIT64
#include "dev-malloc.c"
#endif
#endif

#ifdef ENABLE_ERROR
#include "dev-error.c"
#endif

#ifdef ENABLE_BLOCKS
#include "dev-blocks.c"
#endif

#include "dev-files.c"

#ifdef ENABLE_MULTICORE
#include "dev-multicore.c"
#endif

#ifdef ENABLE_FFI
#include "dev-ffi.c"
#endif

#ifdef ENABLE_FLOATS
#include "dev-float.c"
#endif

#ifdef ENABLE_CLOCK
#include "dev-clock.c"
#endif

#ifdef ENABLE_RNG
#include "dev-rng.c"
#endif

#ifdef ENABLE_SOCKETS
#include "dev-sockets.c"
#endif

#ifdef ENABLE_UNIX
#include "dev-unix.c"
#endif


/*---------------------------------------------------------------------
  Now on to I/O and extensions!
  ---------------------------------------------------------------------*/
#ifdef USE_UTF32
V display_utf8(const unsigned char* utf8_bytes, int num_bytes) {
    if (write(STDOUT_FILENO, utf8_bytes, num_bytes) == -1) {
        perror("Error writing to /dev/stdout");
    }
}

V utf32_to_utf8(uint32_t utf32_char, unsigned char* utf8_bytes, int* num_bytes) {
    if (utf32_char < 0x80) {
        utf8_bytes[0] = (unsigned char)utf32_char;
        *num_bytes = 1;
    } else if (utf32_char < 0x800) {
        utf8_bytes[0] = (unsigned char)(0xC0 | (utf32_char >> 6));
        utf8_bytes[1] = (unsigned char)(0x80 | (utf32_char & 0x3F));
        *num_bytes = 2;
    } else if (utf32_char < 0x10000) {
        utf8_bytes[0] = (unsigned char)(0xE0 | (utf32_char >> 12));
        utf8_bytes[1] = (unsigned char)(0x80 | ((utf32_char >> 6) & 0x3F));
        utf8_bytes[2] = (unsigned char)(0x80 | (utf32_char & 0x3F));
        *num_bytes = 3;
    } else if (utf32_char < 0x110000) {
        utf8_bytes[0] = (unsigned char)(0xF0 | (utf32_char >> 18));
        utf8_bytes[1] = (unsigned char)(0x80 | ((utf32_char >> 12) & 0x3F));
        utf8_bytes[2] = (unsigned char)(0x80 | ((utf32_char >> 6) & 0x3F));
        utf8_bytes[3] = (unsigned char)(0x80 | (utf32_char & 0x3F));
        *num_bytes = 4;
    } else {
        *num_bytes = 0;
    }
}
#endif

V io_output(NgaState *vm) {
#ifdef USE_UTF32
  unsigned char utf8_bytes[4];
  int num_bytes;
  utf32_to_utf8(stack_pop(vm), utf8_bytes, &num_bytes);
  display_utf8(utf8_bytes, num_bytes);
#else
  putc(stack_pop(vm), stdout);
#endif
  fflush(stdout);
}

V query_output(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_OUTPUT);
}


/*=====================================================================*/

#ifdef USE_UTF32
int read_character(int from) {
  unsigned char utf8_bytes[4] = { 0 };
  int utf32_char, i, num_bytes;

  if (read(from, &utf8_bytes[0], 1) != 1) { return 0; }
  if ((utf8_bytes[0] & 0x80) == 0x00) {
    num_bytes = 1;
  } else if ((utf8_bytes[0] & 0xE0) == 0xC0) {
    num_bytes = 2;
  } else if ((utf8_bytes[0] & 0xF0) == 0xE0) {
    num_bytes = 3;
  } else if ((utf8_bytes[0] & 0xF8) == 0xF0) {
    num_bytes = 4;
  } else {
    return 0;
  }

  for (i = 1; i < num_bytes; i++) {
    if (read(from, &utf8_bytes[i], 1) != 1) {
      return 0;
    }
  }

  if (num_bytes == 1) {
    utf32_char = utf8_bytes[0];
  } else if (num_bytes == 2) {
    utf32_char = ((uint32_t)(utf8_bytes[0] & 0x1F) << 6) |
                            (utf8_bytes[1] & 0x3F);
  } else if (num_bytes == 3) {
    utf32_char = ((uint32_t)(utf8_bytes[0] & 0x0F) << 12) |
                 ((uint32_t)(utf8_bytes[1] & 0x3F) << 6) |
                            (utf8_bytes[2] & 0x3F);
  } else if (num_bytes == 4) {
    utf32_char = ((uint32_t)(utf8_bytes[0] & 0x07) << 18) |
                 ((uint32_t)(utf8_bytes[1] & 0x3F) << 12) |
                 ((uint32_t)(utf8_bytes[2] & 0x3F) << 6) |
                            (utf8_bytes[3] & 0x3F);
  } else {
    return 0;
  }
  return utf32_char;
}

int fread_character(FILE *from) {
  unsigned char utf8_bytes[4] = { 0 };
  int utf32_char, i, num_bytes;

  if (fread(&utf8_bytes[0], 1, 1, from) != 1) { return 0; }
  if ((utf8_bytes[0] & 0x80) == 0x00) {
    num_bytes = 1;
  } else if ((utf8_bytes[0] & 0xE0) == 0xC0) {
    num_bytes = 2;
  } else if ((utf8_bytes[0] & 0xF0) == 0xE0) {
    num_bytes = 3;
  } else if ((utf8_bytes[0] & 0xF8) == 0xF0) {
    num_bytes = 4;
  } else {
    return 0;
  }

  for (i = 1; i < num_bytes; i++) {
    if (fread(&utf8_bytes[i], 1, 1, from) != 1) {
      return 0;
    }
  }

  if (num_bytes == 1) {
    utf32_char = utf8_bytes[0];
  } else if (num_bytes == 2) {
    utf32_char = ((uint32_t)(utf8_bytes[0] & 0x1F) << 6) |
                            (utf8_bytes[1] & 0x3F);
  } else if (num_bytes == 3) {
    utf32_char = ((uint32_t)(utf8_bytes[0] & 0x0F) << 12) |
                 ((uint32_t)(utf8_bytes[1] & 0x3F) << 6) |
                            (utf8_bytes[2] & 0x3F);
  } else if (num_bytes == 4) {
    utf32_char = ((uint32_t)(utf8_bytes[0] & 0x07) << 18) |
                 ((uint32_t)(utf8_bytes[1] & 0x3F) << 12) |
                 ((uint32_t)(utf8_bytes[2] & 0x3F) << 6) |
                            (utf8_bytes[3] & 0x3F);
  } else {
    return 0;
  }
  return utf32_char;
}
#endif

V io_keyboard(NgaState *vm) {
#ifdef USE_UTF32
  stack_push(vm, read_character(STDIN_FILENO));
#else
  stack_push(vm, getc(stdin));
#endif
  if (TOS == 127) TOS = 8;
}

V query_keyboard(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_KEYBOARD);
}

/*=====================================================================*/

#ifdef ENABLE_UNSIGNED
V io_unsigned(NgaState *vm) {
  int x, y, z;
  long c;
  switch (stack_pop(vm)) {
    case 0: ACTIVE.u = 1; break;
    case 1:
      c = 0;
      z = stack_pop(vm);
      y = stack_pop(vm);
      x = stack_pop(vm);
      if (ACTIVE.u != 0) {
        c = (unsigned)x * (unsigned)y;
        stack_push(vm, (unsigned)c % (unsigned)z);
        stack_push(vm, (unsigned)c / (unsigned)z);
      }
      else {
        c = x * y;
        stack_push(vm, c % z);
        stack_push(vm, c / z);
      }
      ACTIVE.u = 0;
      break;
  }
}

V query_unsigned(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_UNSIGNED);
}
#endif

/*=====================================================================*/

V io_image(NgaState *vm) {
  FILE *fp;
  char *f = string_extract(vm, stack_pop(vm));
  if ((fp = fopen(f, "wb")) == NULL) {
    printf("\nERROR (nga/io_image): Unable to save the image: %s!\n", f);
    exit(2);
  }
  fwrite(vm->memory, sizeof(CELL), vm->memory[3] + 1, fp);
  fclose(fp);
}

V query_image(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_IMAGE);
}


/*=====================================================================*/


/*---------------------------------------------------------------------
  Scripting Support
  ---------------------------------------------------------------------*/

V scripting_arg(NgaState *vm) {
  CELL a, b;
  a = stack_pop(vm);
  b = stack_pop(vm);
  stack_push(vm, string_inject(vm, vm->sys_argv[a + 2], b));
}

V scripting_arg_count(NgaState *vm) {
  stack_push(vm, vm->sys_argc - 2);
}

V scripting_include(NgaState *vm) {
  include_file(vm, string_extract(vm, stack_pop(vm)), 0);
}

V scripting_name(NgaState *vm) {
  stack_push(vm, string_inject(vm, vm->sys_argv[1], stack_pop(vm)));
}

/* addeded in scripting i/o device, revision 1 */
V scripting_source(NgaState *vm) {
  stack_push(vm, string_inject(vm, vm->scripting_sources[vm->current_source], stack_pop(vm)));
}

V scripting_line(NgaState *vm) {
  stack_push(vm, vm->currentLine);
}

V scripting_ignore_to_eol(NgaState *vm) {
  vm->ignoreToEOL = -1;
}

V scripting_ignore_to_eof(NgaState *vm) {
  vm->ignoreToEOF = -1;
}

V scripting_abort(NgaState *vm) {
  scripting_ignore_to_eol(vm);
  scripting_ignore_to_eof(vm);
  vm->perform_abort = -1;
}

V carry_out_abort(NgaState *vm) {
  ACTIVE.ip = IMAGE_SIZE + 1;
  ACTIVE.rp = 0;
  ACTIVE.sp = 0;
#ifdef ENABLE_FLOATS
  vm->fsp = 0;
  vm->afsp = 0;
#endif

  if (vm->current_source > 0) {
    scripting_abort(vm);
    return;
  }

  vm->perform_abort = 0;
  vm->current_source = 0;
}

V scripting_line_text(NgaState *vm) {
  CELL target = stack_pop(vm);
  string_inject(vm, vm->line, target);
}

Handler ScriptingActions[] = {
  scripting_arg_count,     scripting_arg,
  scripting_include,       scripting_name,
  scripting_source,        scripting_line,
  scripting_ignore_to_eol, scripting_ignore_to_eof,
  scripting_abort,         scripting_line_text
};

V query_scripting(NgaState *vm) {
  stack_push(vm, 2);
  stack_push(vm, DEVICE_SCRIPTING);
}

V io_scripting(NgaState *vm) {
  ScriptingActions[stack_pop(vm)](vm);
}


/*=====================================================================*/

/*---------------------------------------------------------------------
  With these out of the way, I implement `execute`, which takes an
  address and runs the code at it. This has a couple of interesting
  bits.

  This will also exit if the address stack depth is zero (meaning that
  the word being run, and it's dependencies) are finished.
  ---------------------------------------------------------------------*/

V invalid_opcode(NgaState *vm, CELL opcode) {
  CELL a, i;
  printf("\nERROR (nga/execute): Invalid instruction!\n");
  printf("At %lld, opcode %lld\n", (long long)ACTIVE.ip, (long long)opcode);
  printf("Instructions: ");
  a = opcode;
  for (i = 0; i < 4; i++) {
    printf("%lldd ", (long long)a & 0xFF);
    a = a >> 8;
  }
  printf("\n");
  exit(1);
}

V execute(NgaState *vm, CELL cell) {
  CELL opcode;
  if (ACTIVE.rp == 0)
    ACTIVE.rp = 1;
  ACTIVE.ip = cell;
  while (ACTIVE.ip < IMAGE_SIZE) {
    if (vm->perform_abort == 0) {
      opcode = vm->memory[ACTIVE.ip];
      validate_opcode_bundle(vm, opcode);
      process_opcode_bundle(vm, opcode);
#ifndef ENABLE_ERROR
      if (ACTIVE.sp < 0 || ACTIVE.sp > STACK_DEPTH) {
        printf("\nERROR (nga/execute): Stack Limits Exceeded!\n");
        printf("At %lld, opcode %lld. sp = %lld, core = %lld\n", (long long)ACTIVE.ip, (long long)opcode, (long long)ACTIVE.sp, (long long)vm->active);
        exit(1);
      }
      if (ACTIVE.rp < 0 || ACTIVE.rp > ADDRESSES) {
        printf("\nERROR (nga/execute): Address Stack Limits Exceeded!\n");
        printf("At %lld, opcode %lld. rp = %lld\n", (long long)ACTIVE.ip, (long long)opcode, (long long)ACTIVE.rp);
        exit(1);
      }
#endif
      ACTIVE.ip++;
#ifdef ENABLE_MULTICORE
      switch_core(vm);
#endif
      if (ACTIVE.rp == 0)
        ACTIVE.ip = IMAGE_SIZE;
    } else {
      carry_out_abort(vm);
    }
  }
}

;
/*---------------------------------------------------------------------
  RETRO's `interpret` word expects a token on the stack. This next
  function copies a token to the `TIB` (text input buffer) and then
  calls `interpret` to process it.
  ---------------------------------------------------------------------*/

V evaluate(NgaState *vm, char *s) {
  if (strlen(s) == 0)  return;
  string_inject(vm, s, TIB);
  stack_push(vm, TIB);
  execute(vm, vm->interpret);
}


/*---------------------------------------------------------------------
  `read_token` reads a token from the specified file.  It will stop on
   a whitespace or newline. It also tries to handle backspaces, though
   the success of this depends on how your terminal is configured.
  ---------------------------------------------------------------------*/

int not_eol(int c) {
  return (c != 9) && (c != 10) && (c != 13) && (c != 32) && (c != EOF) && (c != 0);
}

V read_token(FILE *file, char *token_buffer) {
#ifdef USE_UTF32
  int ch = fread_character(file);
#else
  int ch = getc(file);
#endif
  int count = 0;
  while (not_eol(ch)) {
    if ((ch == 8 || ch == 127) && count > 0) {
      count--;
    } else {
      token_buffer[count++] = ch;
    }
#ifdef USE_UTF32
    ch = fread_character(file);
#else
    ch = getc(file);
#endif
  }
  token_buffer[count] = '\0';
}


V skip_indent(FILE *fp) {
  int ch = getc(fp);
  while (ch == ' ') {
    ch = getc(fp);
  }
  ungetc(ch, fp);
}

/*---------------------------------------------------------------------
  Display the Stack Contents
  ---------------------------------------------------------------------*/

V dump_stack(NgaState *vm) {
  if (ACTIVE.sp == 0)  return;
  printf("\nStack: ");
  for (CELL i = 1; i <= ACTIVE.sp; i++) {
    if (i == ACTIVE.sp)
      printf("[ TOS: %lld ]", (long long)ACTIVE.data[i]);
    else
      printf("%lld ", (long long)ACTIVE.data[i]);
  }
  printf("\n");
}

V dump_astack(NgaState *vm) {
  if (ACTIVE.rp == 0)  return;
  printf("\nAddress Stack: ");
  for (CELL i = 1; i <= ACTIVE.rp; i++) {
    if (i == ACTIVE.rp)
      printf("[ TOS: %lld ]", (long long)ACTIVE.address[i]);
    else
      printf("%lld ", (long long)ACTIVE.address[i]);
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

int fence_boundary(NgaState *vm, char *buffer, int tests_enabled) {
  int flag = 1;
  if (strcmp(buffer, vm->code_start) == 0) { flag = -1; }
  if (strcmp(buffer, vm->code_end) == 0)   { flag = -1; }
  if (strcmp(buffer, vm->test_start) == 0) {
    if (vm->codeBlocks == 0) { vm->codeBlocks++; }
  }
  if (tests_enabled == 0) { return flag; }
  if (strcmp(buffer, vm->test_start) == 0) { flag = -1; }
  if (strcmp(buffer, vm->test_end) == 0)   { flag = -1; }
  return flag;
}


/*---------------------------------------------------------------------
  And now for the actual `include_file()` function.
  ---------------------------------------------------------------------*/

V read_line(NgaState *vm, FILE *file, char *token_buffer) {
  int ch = getc(file);
  int count = 0;
  token_buffer[0] = '\0';
  while ((ch != 10) && (ch != 13) && (ch != EOF) && (ch != 0)) {
    token_buffer[count++] = ch;
#ifdef USE_UTF32
    ch = fread_character(file);
#else
    ch = getc(file);
#endif
  }
  token_buffer[count] = '\0';
}

int count_tokens(char *line) {
  int count = 1;
  while (*line++) {
    if (isspace(line[0]))
      count++;
  }
  return count;
}

V include_file(NgaState *vm, char *fname, int run_tests) {
  int inBlock = 0;                 /* Tracks status of in/out of block */
  int priorBlocks = 0;
  char source[64 * 1024];          /* Token buffer [about 64K]         */
  char fence[33];                  /* Used with `fence_boundary()`     */

  CELL ReturnStack[ADDRESSES];
  CELL arp, aip;

  long offset = 0;
  CELL at = 0;
  int tokens = 0;
  FILE *fp;                        /* Open the file. If not found,     */
  fp = fopen(fname, "r");          /* exit.                            */
  if (fp == NULL) {
    printf("File `%s` not found. Exiting.\n", fname);
    exit(1);
  }

  priorBlocks = vm->codeBlocks;
  vm->codeBlocks = 0;

  arp = ACTIVE.rp;
  aip = ACTIVE.ip;
  for(ACTIVE.rp = 0; ACTIVE.rp <= arp; ACTIVE.rp++)
    ReturnStack[ACTIVE.rp] = ACTIVE.address[ACTIVE.rp];
  ACTIVE.rp = 0;

  vm->current_source++;
  strlcpy(vm->scripting_sources[vm->current_source], fname, 8192);

  vm->ignoreToEOF = 0;

  while (!feof(fp) && (vm->ignoreToEOF == 0)) { /* Loop through the file   */

    vm->ignoreToEOL = 0;

    offset = ftell(fp);
    read_line(vm, fp, vm->line);
    at++;
    fseek(fp, offset, SEEK_SET);
    skip_indent(fp);

    tokens = count_tokens(vm->line);

    while (tokens > 0 && vm->ignoreToEOL == 0) {
      tokens--;
      read_token(fp, source);
      strlcpy(fence, source, 32); /* Copy the first three characters  */
      if (fence_boundary(vm, fence, run_tests) == -1) {
        if (inBlock == 0) {
          inBlock = 1;
          vm->codeBlocks++;
        } else {
          inBlock = 0;
        }
      } else {
        if (inBlock == 1) {
          vm->currentLine = at;
          evaluate(vm, source);
          vm->currentLine = at;
        }
      }
    }
    if (vm->ignoreToEOL == -1) {
      read_line(vm, fp, vm->line);
    }
  }

  vm->current_source--;
  vm->ignoreToEOF = 0;
  fclose(fp);
  if (vm->perform_abort == -1) {
    carry_out_abort(vm);
  }
  for(ACTIVE.rp = 0; ACTIVE.rp <= arp; ACTIVE.rp++)
    ACTIVE.address[ACTIVE.rp] = ReturnStack[ACTIVE.rp];
  ACTIVE.rp = arp;
  ACTIVE.ip = aip;

  if (vm->codeBlocks == 0) {
    printf("warning: no code or test blocks found!\n");
    printf("         filename: %s\n", fname);
    printf("         see http://unu.retroforth.org for a brief summary of\n");
    printf("         the unu code format used by retro\n");

  }
  vm->codeBlocks = priorBlocks;
}


/*---------------------------------------------------------------------
  `initialize()` sets up Nga and loads the image (from the array in
  `image.c`) to memory.
  ---------------------------------------------------------------------*/

V initialize(NgaState *vm) {
  prepare_vm(vm);
  load_embedded_image(vm);
  vm->interactive = 0;

  strlcpy(vm->code_start, "~~~", 256);
  strlcpy(vm->code_end,   "~~~", 256);
  strlcpy(vm->test_start, "```", 256);
  strlcpy(vm->test_end,   "```", 256);

  /* Setup variables related to the scripting device */
  vm->currentLine = 0;           /* Current Line # for script */
  vm->current_source = 0;        /* Current file being run    */
  vm->perform_abort = 0;         /* Carry out abort procedure */
  strlcpy(vm->scripting_sources[0], "/dev/stdin", 8192);
  vm->ignoreToEOL = 0;
  vm->ignoreToEOF = 0;
  vm->codeBlocks = 0;
}


V help(char *exename) {
  printf("Scripting Usage: %s filename\n\n", exename);
  printf("Interactive Usage: %s [-h] [-i] [-f filename] [-t filename]\n\n", exename);
  printf("Valid Arguments:\n\n");
  printf("  -h\n");
  printf("    Display this help text\n");
  printf("  -i\n");
  printf("    Launches in interactive mode\n");
  printf("  -f filename\n");
  printf("    Run the contents of the specified file\n");
  printf("  -u filename\n");
  printf("    Use the image in the specified file instead of the internal one\n");
  printf("  -r filename\n");
  printf("    Use the image in the specified file instead of the internal one and run the code in it\n");
  printf("  -t filename\n");
  printf("    Run the contents of the specified file, including any tests (in ``` blocks)\n\n");
  printf("  -v\n");
  printf("    Run in verbose mode\n");
}

/* Signal Handler -----------------------------------------------------*/

#ifdef ENABLE_SIGNALS
static V sig_handler(int _)
{
  printf("\nCaught: %d\n", _);
  exit(1);
}
#endif

/* Main Entry Point ---------------------------------------------------*/
enum flags {
  FLAG_HELP, FLAG_INTERACTIVE,
};

V register_devices(NgaState *vm) {
  register_device(vm, io_output, query_output);
  register_device(vm, io_keyboard, query_keyboard);
  register_device(vm, io_filesystem, query_filesystem);
  register_device(vm, io_image, query_image);
#ifdef ENABLE_FLOATS
  register_device(vm, io_floatingpoint, query_floatingpoint);
#endif
#ifdef ENABLE_UNIX
  register_device(vm, io_unix, query_unix);
#endif
#ifdef ENABLE_MALLOC
#ifdef BIT64
  register_device(vm, io_malloc, query_malloc);
#endif
#endif
#ifdef ENABLE_BLOCKS
  register_device(vm, io_blocks, query_blocks);
#endif
#ifdef ENABLE_CLOCK
  register_device(vm, io_clock, query_clock);
#endif
  register_device(vm, io_scripting, query_scripting);
#ifdef ENABLE_RNG
  register_device(vm, io_rng, query_rng);
#endif
#ifdef ENABLE_SOCKETS
  register_device(vm, io_socket, query_socket);
#endif
#ifdef ENABLE_MULTICORE
  register_device(vm, io_multicore, query_multicore);
#endif
#ifdef ENABLE_FFI
  register_device(vm, io_ffi, query_ffi);
  nlibs = 0;
  nffi = 0;
#endif
#ifdef ENABLE_UNSIGNED
  register_device(vm, io_unsigned, query_unsigned);
#endif
#ifdef ENABLE_ERROR
  register_device(vm, io_error, query_error);
#endif
}

V register_signal_handlers() {
#ifdef ENABLE_SIGNALS
  signal(SIGHUP, sig_handler);
  signal(SIGINT, sig_handler);
  signal(SIGILL, sig_handler);
  signal(SIGBUS, sig_handler);
  signal(SIGFPE, sig_handler);
#endif
}

#define ARG(n) (strcmp(argv[i], n) == 0)

int main(int argc, char **argv) {
  int i;
  int modes[16];
  NgaState *vm = calloc(sizeof(NgaState), sizeof(char));
  verbose = 0;

  register_signal_handlers();

  initialize(vm);               /* Initialize Nga & image    */
  register_devices(vm);
  vm->sys_argc = argc;           /* Point the global argc and */
  vm->sys_argv = argv;           /* argv to the actual ones   */

  /* Check arguments. If no flags were passed, load & run the
     file specified and exit. */
  if (argc >= 2 && argv[1][0] != '-') {
    update_rx(vm);
    include_file(vm, argv[1], 0);
    if (ACTIVE.sp >= 1)  dump_stack(vm);
    exit(0);
  }

  /* Clear startup modes       */
  for (i = 0; i < 16; i++)
    modes[i] = 0;

  if (argc <= 1) modes[FLAG_INTERACTIVE] = 1;

  update_rx(vm);

  /* Process Arguments */
  for (i = 1; i < argc; i++) {
    if  ARG("-h") {
      help(argv[0]);
      exit(0);
    } else if ARG("-v") {
      verbose = 1;
    } else if ARG("-i") {
      modes[FLAG_INTERACTIVE] = 1;
      vm->interactive = -1;
    } else if ARG("-f") {
      include_file(vm, argv[i + 1], 0);
      i++;
    } else if ARG("-u") {
      i++;
      load_image(vm, argv[i]);
      update_rx(vm);
    } else if ARG("-r") {
      i++;
      load_image(vm, argv[i]);
      modes[FLAG_INTERACTIVE] = 1;
      update_rx(vm);
    } else if ARG("-t") {
      include_file(vm, argv[i + 1], 1);
      i++;
    } else  if (ARG("--code-start") || ARG("-cs")) {
      i++;
      strlcpy(vm->code_start, argv[i], 256);
    } else if (ARG("--code-end") || ARG("-ce")) {
      i++;
      strlcpy(vm->code_end, argv[i], 256);
    } else if (ARG("--test-start") || ARG("-ts")) {
      i++;
      strlcpy(vm->test_start, argv[i], 256);
    } else if (ARG("--test-end") || ARG("-te")) {
      i++;
      strlcpy(vm->test_end, argv[i], 256);
    }
  }

  /* Run the Listener (if interactive mode was set) */
  if (modes[FLAG_INTERACTIVE] == 1) {
    execute(vm, 0);
  }

  /* Dump Stack */
  if (ACTIVE.sp >= 1)  dump_stack(vm);

  free(vm);
}


/*=====================================================================*/


/*---------------------------------------------------------------------
  Interfacing With The Image
  ---------------------------------------------------------------------*/

/*---------------------------------------------------------------------
  Stack push/pop is easy. I could avoid these, but it aids in keeping
  the code readable, so it's worth the slight overhead.
  ---------------------------------------------------------------------*/

CELL stack_pop(NgaState *vm) {
  ACTIVE.sp--;
  return ACTIVE.data[ACTIVE.sp + 1];
}

V stack_push(NgaState *vm, CELL value) {
  ACTIVE.sp++;
  ACTIVE.data[ACTIVE.sp] = value;
}


/*---------------------------------------------------------------------
  Strings are next. RETRO uses C-style NULL terminated strings. So I
  can easily inject or extract a string. Injection iterates over the
  string, copying it into the image. This also takes care to ensure
  that the NULL terminator is added.
  ---------------------------------------------------------------------*/

CELL string_inject(NgaState *vm, char *str, CELL buffer) {
  if (!str) {
    vm->memory[buffer] = 0;
    return 0;
  }
  for (CELL i = 0; str[i] != '\0'; i++) {
    vm->memory[buffer + i] = (CELL)str[i];
    vm->memory[buffer + i + 1] = 0;
  }
  return buffer;
}


/*---------------------------------------------------------------------
  Extracting a string is similar, but I have to iterate over the VM
  memory instead of a C string and copy the charaters into a buffer.
  This uses a static buffer (`string_data`) as I prefer to avoid using
  `malloc()`.
  ---------------------------------------------------------------------*/

char *string_extract(NgaState *vm, CELL at) {
  CELL starting = at, i = 0;
  while (vm->memory[starting] && i < 8192)
    vm->string_data[i++] = (char)vm->memory[starting++];
  vm->string_data[i] = 0;
  return (char *)vm->string_data;
}

/*---------------------------------------------------------------------
  This interface tracks a few words and variables in the image. These
  are:

  Dictionary - the latest dictionary header
  interpret  - the heart of the interpreter/compiler

  I have to call this periodically, as the Dictionary will change as
  new words are defined, and the user might write a new error handler
  or interpreter.
  ---------------------------------------------------------------------*/

V update_rx(NgaState *vm) {
  vm->Dictionary = vm->memory[2];
  vm->interpret = vm->memory[5];
  if (vm->memory[10] != 0) { execute(vm, vm->memory[10]); }
}

/*=====================================================================*/

V register_device(NgaState *vm, V *handler, V *query) {
  vm->IO_deviceHandlers[vm->devices] = handler;
  vm->IO_queryHandlers[vm->devices] = query;
  vm->devices++;
}

V load_embedded_image(NgaState *vm) {
  int i;
  for (i = 0; i < ngaImageCells; i++)
    vm->memory[i] = ngaImage[i];
}

CELL load_image(NgaState *vm, char *imageFile) {
  FILE *fp;
  CELL imageSize = 0;
  long fileLen;
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
    for (int i = 0; i < IMAGE_SIZE; i++)
      vm->memory[i] = 0; /* NO - nop instruction */
    /* Read the file into memory */
    imageSize = fread(vm->memory, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
  return imageSize;
}

V prepare_vm(NgaState *vm) {
  vm->active = 0;
  ACTIVE.ip = ACTIVE.sp = ACTIVE.rp = ACTIVE.u = 0;
  ACTIVE.active = -1;
  for (ACTIVE.ip = 0; ACTIVE.ip < IMAGE_SIZE; ACTIVE.ip++)
    vm->memory[ACTIVE.ip] = 0; /* NO - nop instruction */
  for (ACTIVE.ip = 0; ACTIVE.ip < STACK_DEPTH; ACTIVE.ip++)
    ACTIVE.data[ACTIVE.ip] = 0;
  for (ACTIVE.ip = 0; ACTIVE.ip < ADDRESSES; ACTIVE.ip++)
    ACTIVE.address[ACTIVE.ip] = 0;
}

V inst_no(NgaState *vm) {
  guard(vm, 0, 0, 0);
}

V inst_li(NgaState *vm) {
  guard(vm, 0, 1, 0);
  ACTIVE.sp++;
  ACTIVE.ip++;
  TOS = vm->memory[ACTIVE.ip];
}

V inst_du(NgaState *vm) {
  guard(vm, 1, 2, 0);
  ACTIVE.sp++;
  ACTIVE.data[ACTIVE.sp] = NOS;
}

V inst_dr(NgaState *vm) {
  guard(vm, 1, 0, 0);
  ACTIVE.data[ACTIVE.sp] = 0;
  ACTIVE.sp--;
}

V inst_sw(NgaState *vm) {
  guard(vm, 2, 2, 0);
  CELL a;
  a = TOS;
  TOS = NOS;
  NOS = a;
}

V inst_pu(NgaState *vm) {
  guard(vm, 1, 0, 1);
  ACTIVE.rp++;
  TORS = TOS;
  inst_dr(vm);
}

V inst_po(NgaState *vm) {
  guard(vm, 0, 1, -1);
  ACTIVE.sp++;
  TOS = TORS;
  ACTIVE.rp--;
}

V inst_ju(NgaState *vm) {
  guard(vm, 1, 0, 0);
  ACTIVE.ip = TOS - 1;
  inst_dr(vm);
}

V inst_ca(NgaState *vm) {
  guard(vm, 1, 0, 1);
  ACTIVE.rp++;
  TORS = ACTIVE.ip;
  ACTIVE.ip = TOS - 1;
  inst_dr(vm);
}

V inst_cc(NgaState *vm) {
  guard(vm, 2, 0, 1);
  CELL a, b;
  a = TOS; inst_dr(vm);  /* Target */
  b = TOS; inst_dr(vm);  /* Flag   */
  if (b != 0) {
    ACTIVE.rp++;
    TORS = ACTIVE.ip;
    ACTIVE.ip = a - 1;
  }
}

V inst_re(NgaState *vm) {
  guard(vm, 0, 0, -1);
  ACTIVE.ip = TORS;
  ACTIVE.rp--;
}

V inst_eq(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = ((unsigned)NOS == (unsigned)TOS) ? -1 : 0;
    ACTIVE.u = 0;
  } else {
    NOS = (NOS == TOS) ? -1 : 0;
  }
  inst_dr(vm);
}

V inst_ne(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = ((unsigned)NOS != (unsigned)TOS) ? -1 : 0;
    ACTIVE.u = 0;
  } else {
    NOS = (NOS != TOS) ? -1 : 0;
  }
  inst_dr(vm);
}

V inst_lt(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = ((unsigned)NOS < (unsigned)TOS) ? -1 : 0;
    ACTIVE.u = 0;
  } else {
    NOS = (NOS < TOS) ? -1 : 0;
  }
  inst_dr(vm);
}

V inst_gt(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = ((unsigned)NOS > (unsigned)TOS) ? -1 : 0;
    ACTIVE.u = 0;
  } else {
    NOS = (NOS > TOS) ? -1 : 0;
  }
  inst_dr(vm);
}

V inst_fe(NgaState *vm) {
  guard(vm, 1, 1, 0);
  switch (TOS) {
    case -1: TOS = ACTIVE.sp - 1; break;
    case -2: TOS = ACTIVE.rp; break;
    case -3: TOS = IMAGE_SIZE; break;
    case -4: TOS = CELL_MIN; break;
    case -5: TOS = CELL_MAX; break;
    default: TOS = vm->memory[TOS]; break;
  }
}

V inst_st(NgaState *vm) {
  guard(vm, 2, 0, 0);
  vm->memory[TOS] = NOS;
  inst_dr(vm);
  inst_dr(vm);
}

V inst_ad(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = (unsigned)NOS + (unsigned)TOS;
    ACTIVE.u = 0;
  } else {
    NOS += TOS;
  }
  inst_dr(vm);
}

V inst_su(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = (unsigned)NOS - (unsigned)TOS;
    ACTIVE.u = 0;
  } else {
    NOS -= TOS;
  }
  inst_dr(vm);
}

V inst_mu(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = (unsigned)NOS * (unsigned)TOS;
    ACTIVE.u = 0;
  } else {
    NOS *= TOS;
  }
  inst_dr(vm);
}

V inst_di(NgaState *vm) {
  guard(vm, 2, 2, 0);
  CELL a, b;
  a = TOS;
  b = NOS;
  if (b == 0) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[6] != 0) {
    }
#endif
  }
  if (ACTIVE.u != 0) {
    TOS = (unsigned)b / (unsigned)a;
    NOS = (unsigned)b % (unsigned)a;
    ACTIVE.u = 0;
  } else {
    TOS = b / a;
    NOS = b % a;
  }
}

V inst_an(NgaState *vm) {
  guard(vm, 2, 1, 0);
  NOS = TOS & NOS;
  inst_dr(vm);
}

V inst_or(NgaState *vm) {
  guard(vm, 2, 1, 0);
  NOS = TOS | NOS;
  inst_dr(vm);
}

V inst_xo(NgaState *vm) {
  guard(vm, 2, 1, 0);
  NOS = TOS ^ NOS;
  inst_dr(vm);
}

V inst_sh(NgaState *vm) {
  guard(vm, 2, 1, 0);
  CELL y = TOS;
  CELL x = NOS;
  if (TOS < 0)
    NOS = NOS << (0 - TOS);
  else {
    if (ACTIVE.u != 0) {
      NOS = (unsigned)x >> (unsigned)y;
      ACTIVE.u = 0;
    } else {
      if (x < 0 && y > 0)
        NOS = x >> y | ~(~0U >> y);
      else
        NOS = x >> y;
    }
  }
  inst_dr(vm);
}

V inst_zr(NgaState *vm) {
  guard(vm, 1, 0, 0);
  if (TOS == 0) {
    inst_dr(vm);
    ACTIVE.ip = TORS;
    ACTIVE.rp--;
  }
}

V inst_ha(NgaState *vm) {
  guard(vm, 0, 0, 0);
  ACTIVE.ip = IMAGE_SIZE;
  ACTIVE.rp = 0;
  exit(0);
}

V inst_ie(NgaState *vm) {
  guard(vm, 1, 1, 0);
  stack_push(vm, vm->devices);
}

V inst_iq(NgaState *vm) {
  guard(vm, 1, 1, 0);
  vm->IO_queryHandlers[stack_pop(vm)](vm);
}

V inst_ii(NgaState *vm) {
  guard(vm, 1, 0, 0);
  vm->IO_deviceHandlers[stack_pop(vm)](vm);
}

Handler instructions[] = {
  inst_no, inst_li, inst_du, inst_dr, inst_sw, inst_pu, inst_po,
  inst_ju, inst_ca, inst_cc, inst_re, inst_eq, inst_ne, inst_lt,
  inst_gt, inst_fe, inst_st, inst_ad, inst_su, inst_mu, inst_di,
  inst_an, inst_or, inst_xo, inst_sh, inst_zr, inst_ha, inst_ie,
  inst_iq, inst_ii
};

V process_opcode(NgaState *vm, CELL opcode) {
#ifdef FAST
  switch (opcode) {
    case 0: break;
    case 1: inst_li(vm); break;
    case 2: inst_du(vm); break;
    case 3: inst_dr(vm); break;
    case 4: inst_sw(vm); break;
    case 5: inst_pu(vm); break;
    case 6: inst_po(vm); break;
    case 7: inst_ju(vm); break;
    case 8: inst_ca(vm); break;
    case 9: inst_cc(vm); break;
    case 10: inst_re(vm); break;
    case 11: inst_eq(vm); break;
    case 12: inst_ne(vm); break;
    case 13: inst_lt(vm); break;
    case 14: inst_gt(vm); break;
    case 15: inst_fe(vm); break;
    case 16: inst_st(vm); break;
    case 17: inst_ad(vm); break;
    case 18: inst_su(vm); break;
    case 19: inst_mu(vm); break;
    case 20: inst_di(vm); break;
    case 21: inst_an(vm); break;
    case 22: inst_or(vm); break;
    case 23: inst_xo(vm); break;
    case 24: inst_sh(vm); break;
    case 25: inst_zr(vm); break;
    case 26: inst_ha(vm); break;
    case 27: inst_ie(vm); break;
    case 28: inst_iq(vm); break;
    case 29: inst_ii(vm); break;
    default: break;
  }
#else
  if (opcode != 0)
    instructions[opcode](vm);
#endif
}

V validate_opcode_bundle(NgaState *vm, CELL opcode) {
  CELL remainingOpcode = opcode;
  for (int i = 0; i < 4; i++) {
    CELL current = remainingOpcode & 0xFF;
    if (current < 0 || current > 29) {
      invalid_opcode(vm, opcode);
    }
    remainingOpcode >>= 8;
  }
}

V verbose_details(NgaState *vm, CELL opcode) {
  fprintf(stderr, "ip: %lld ", (long long)ACTIVE.ip);
  fprintf(stderr, "sp: %lld ", (long long)ACTIVE.sp);
  fprintf(stderr, "rp: %lld ", (long long)ACTIVE.rp);
  fprintf(stderr, "core: %lld ", (long long)vm->active);
  fprintf(stderr, "opcode: %lld\n", (long long)opcode);
}

#define INST(n) ((opcode >> n) & 0xFF) != 0
V process_opcode_bundle(NgaState *vm, CELL opcode) {
  if (INST(0))  instructions[opcode & 0xFF](vm);
  if (INST(8))  instructions[(opcode >> 8) & 0xFF](vm);
  if (INST(16)) instructions[(opcode >> 16) & 0xFF](vm);
  if (INST(24)) instructions[(opcode >> 24) & 0xFF](vm);
}

#ifdef NEEDS_STRL
/*---------------------------------------------------------------------
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

size_t strlcat(char *dst, const char *src, size_t dsize) {
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

size_t strlcpy(char *dst, const char *src, size_t dsize) {
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
#endif
