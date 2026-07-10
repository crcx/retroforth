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
#include <stddef.h>
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
#define TIB_END vm->memory[8]

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
V include_plain_file(NgaState *, char *, int);
V initialize_scripting(NgaState *);

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

#ifdef ENABLE_IOCTL
IO(ioctl)
#endif

IO(image)

V load_embedded_image(NgaState *);
CELL load_image(NgaState *, char *);
V prepare_vm(NgaState *);
V execute(NgaState *, CELL);
V process_opcode_bundle(NgaState *, CELL);
#ifndef BRANCH_PREDICTION
V validate_opcode_bundle(NgaState *, CELL);
#endif

#ifdef NEEDS_STRL
size_t strlcat(char *dst, const char *src, size_t dsize);
size_t strlcpy(char *dst, const char *src, size_t dsize);
#endif

V i_no(NgaState *);  V i_li(NgaState *);
V i_du(NgaState *);  V i_dr(NgaState *);
V i_sw(NgaState *);  V i_pu(NgaState *);
V i_po(NgaState *);  V i_ju(NgaState *);
V i_ca(NgaState *);  V i_cc(NgaState *);
V i_re(NgaState *);  V i_eq(NgaState *);
V i_ne(NgaState *);  V i_lt(NgaState *);
V i_gt(NgaState *);  V i_fe(NgaState *);
V i_st(NgaState *);  V i_ad(NgaState *);
V i_su(NgaState *);  V i_mu(NgaState *);
V i_di(NgaState *);  V i_an(NgaState *);
V i_or(NgaState *);  V i_xo(NgaState *);
V i_sh(NgaState *);  V i_zr(NgaState *);
V i_ha(NgaState *);  V i_ie(NgaState *);
V i_iq(NgaState *);  V i_ii(NgaState *);


/* Image, Stack, and VM variables ------------------------------------ */
#define TOS  ACTIVE.data[ACTIVE.sp]
#define NOS  ACTIVE.data[ACTIVE.sp-1]
#define TORS ACTIVE.address[ACTIVE.rp]

/* Global Variables -------------------------------------------------- */

int verbose;

#ifndef BRANCH_PREDICTION
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
#else
V guard(NgaState *vm, int n, int m, int diff) {
  if (unlikely(ACTIVE.sp < n)) {
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
  if (unlikely(((ACTIVE.sp + m) - n) > (STACK_DEPTH - 1))) {
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
  if (unlikely(ACTIVE.rp + diff < 0)) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[3] != 0) {
      handle_error(vm, 3);
    }
#else
      return;
#endif
  }
  if (unlikely(ACTIVE.rp + diff > (ADDRESSES - 1))) {
#ifdef ENABLE_ERROR
    if (vm->ErrorHandlers[1] != 4) {
      handle_error(vm, 4);
    }
#else
    return;
#endif
  }
}
#endif

#include "string_handling.c"

#ifdef ENABLE_IOCTL
#include "dev-ioctl.c"
#endif

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

#ifdef ENABLE_FILES
#include "dev-files.c"
#endif

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


V io_output(NgaState *vm) {
  unsigned char utf8_bytes[4];
  int num_bytes;
  utf32_to_utf8(stack_pop(vm), utf8_bytes, &num_bytes);
  display_utf8(utf8_bytes, num_bytes);
  fflush(stdout);
}

V query_output(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_OUTPUT);
}


/*=====================================================================*/

V io_keyboard(NgaState *vm) {
  stack_push(vm, read_character(STDIN_FILENO));
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


#include "scripting.c"

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
#ifndef BRANCH_PREDICTION
      validate_opcode_bundle(vm, opcode);
#endif
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
  `initialize()` sets up Nga and loads the image (from the array in
  `image.c`) to memory.
  ---------------------------------------------------------------------*/

V initialize(NgaState *vm) {
  prepare_vm(vm);
  load_embedded_image(vm);
  initialize_scripting(vm);
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
  printf("    Run the contents of the code blocks in the specified file\n");
  printf("  -p filename\n");
  printf("    Run the contents of the specified file\n");
  printf("  -u filename\n");
  printf("    Use the image in the specified file instead of the internal one\n");
  printf("  -r filename\n");
  printf("    Use the image in the specified file instead of the internal one and run the code in it\n");
  printf("  -t filename\n");
  printf("    Run the contents of the code blocks in the specified file, including any tests (in ``` blocks)\n\n");
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
#ifdef ENABLE_FILES
  register_device(vm, io_filesystem, query_filesystem);
#endif
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
#ifdef ENABLE_IOCTL
  register_device(vm, io_ioctl, query_ioctl);
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

  strlcpy(vm->scripting_sources[0], "<none>", 8192);


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
    if ARG("-h") {
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
    } else if ARG("-p") {
      include_plain_file(vm, argv[i + 1], 0);
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
    } else if (ARG("--code-start") || ARG("-cs")) {
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

    /* Erase old image in memory: 0 = nop instruction */
    for (int i = 0; i < IMAGE_SIZE; i++) { vm->memory[i] = 0; }

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

V i_no(NgaState *vm) {
#ifndef BRANCH_PREDICTION
  guard(vm, 0, 0, 0);
#endif
}

V i_li(NgaState *vm) {
  guard(vm, 0, 1, 0);
  ACTIVE.sp++;
  ACTIVE.ip++;
  TOS = vm->memory[ACTIVE.ip];
}

V i_du(NgaState *vm) {
  guard(vm, 1, 2, 0);
  ACTIVE.sp++;
  ACTIVE.data[ACTIVE.sp] = NOS;
}

V i_dr(NgaState *vm) {
  guard(vm, 1, 0, 0);
  ACTIVE.data[ACTIVE.sp] = 0;
  ACTIVE.sp--;
}

V i_sw(NgaState *vm) {
  guard(vm, 2, 2, 0);
  CELL a;
  a = TOS;
  TOS = NOS;
  NOS = a;
}

V i_pu(NgaState *vm) {
  guard(vm, 1, 0, 1);
  ACTIVE.rp++;
  TORS = TOS;
  i_dr(vm);
}

V i_po(NgaState *vm) {
  guard(vm, 0, 1, -1);
  ACTIVE.sp++;
  TOS = TORS;
  ACTIVE.rp--;
}

V i_ju(NgaState *vm) {
  guard(vm, 1, 0, 0);
  ACTIVE.ip = TOS - 1;
  i_dr(vm);
}

V i_ca(NgaState *vm) {
  guard(vm, 1, 0, 1);
  ACTIVE.rp++;
  TORS = ACTIVE.ip;
  ACTIVE.ip = TOS - 1;
  i_dr(vm);
}

V i_cc(NgaState *vm) {
  guard(vm, 2, 0, 1);
  CELL a, b;
  a = TOS; i_dr(vm);  /* Target */
  b = TOS; i_dr(vm);  /* Flag   */
  if (b != 0) {
    ACTIVE.rp++;
    TORS = ACTIVE.ip;
    ACTIVE.ip = a - 1;
  }
}

V i_re(NgaState *vm) {
  guard(vm, 0, 0, -1);
  ACTIVE.ip = TORS;
  ACTIVE.rp--;
}

V i_eq(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = ((unsigned)NOS == (unsigned)TOS) ? -1 : 0;
    ACTIVE.u = 0;
  } else {
    NOS = (NOS == TOS) ? -1 : 0;
  }
  i_dr(vm);
}

V i_ne(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = ((unsigned)NOS != (unsigned)TOS) ? -1 : 0;
    ACTIVE.u = 0;
  } else {
    NOS = (NOS != TOS) ? -1 : 0;
  }
  i_dr(vm);
}

V i_lt(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = ((unsigned)NOS < (unsigned)TOS) ? -1 : 0;
    ACTIVE.u = 0;
  } else {
    NOS = (NOS < TOS) ? -1 : 0;
  }
  i_dr(vm);
}

V i_gt(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = ((unsigned)NOS > (unsigned)TOS) ? -1 : 0;
    ACTIVE.u = 0;
  } else {
    NOS = (NOS > TOS) ? -1 : 0;
  }
  i_dr(vm);
}

V i_fe(NgaState *vm) {
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

V i_st(NgaState *vm) {
  guard(vm, 2, 0, 0);
  vm->memory[TOS] = NOS;
  i_dr(vm);
  i_dr(vm);
}

V i_ad(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = (unsigned)NOS + (unsigned)TOS;
    ACTIVE.u = 0;
  } else {
    NOS += TOS;
  }
  i_dr(vm);
}

V i_su(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = (unsigned)NOS - (unsigned)TOS;
    ACTIVE.u = 0;
  } else {
    NOS -= TOS;
  }
  i_dr(vm);
}

V i_mu(NgaState *vm) {
  guard(vm, 2, 1, 0);
  if (ACTIVE.u != 0) {
    NOS = (unsigned)NOS * (unsigned)TOS;
    ACTIVE.u = 0;
  } else {
    NOS *= TOS;
  }
  i_dr(vm);
}

V i_di(NgaState *vm) {
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

V i_an(NgaState *vm) {
  guard(vm, 2, 1, 0);
  NOS = TOS & NOS;
  i_dr(vm);
}

V i_or(NgaState *vm) {
  guard(vm, 2, 1, 0);
  NOS = TOS | NOS;
  i_dr(vm);
}

V i_xo(NgaState *vm) {
  guard(vm, 2, 1, 0);
  NOS = TOS ^ NOS;
  i_dr(vm);
}

V i_sh(NgaState *vm) {
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
  i_dr(vm);
}

V i_zr(NgaState *vm) {
  guard(vm, 1, 0, 0);
  if (TOS == 0) {
    i_dr(vm);
    ACTIVE.ip = TORS;
    ACTIVE.rp--;
  }
}

V i_ha(NgaState *vm) {
  guard(vm, 0, 0, 0);
  ACTIVE.ip = IMAGE_SIZE;
  ACTIVE.rp = 0;
  exit(0);
}

V i_ie(NgaState *vm) {
  guard(vm, 1, 1, 0);
  stack_push(vm, vm->devices);
}

int valid_device(NgaState *vm, CELL device) {
  return device >= 0 && device < vm->devices && device < MAX_DEVICES;
}

V invalid_device(NgaState *vm, CELL device) {
#ifdef ENABLE_ERROR
  if (vm->ErrorHandlers[5] != 0) {
    handle_error(vm, 5);
    return;
  }
#endif
  printf("\nERROR (nga/device): Invalid device id %lld\n", (long long)device);
  ACTIVE.ip = IMAGE_SIZE;
  ACTIVE.rp = 0;
}

V i_iq(NgaState *vm) {
  guard(vm, 1, 1, 0);
  CELL device = stack_pop(vm);
  if (valid_device(vm, device)) {
    vm->IO_queryHandlers[device](vm);
  } else {
    invalid_device(vm, device);
  }
}

V i_ii(NgaState *vm) {
  guard(vm, 1, 0, 0);
  CELL device = stack_pop(vm);
  if (valid_device(vm, device)) {
    vm->IO_deviceHandlers[device](vm);
  } else {
    invalid_device(vm, device);
  }
}

Handler instructions[] = {
  i_no, i_li, i_du, i_dr, i_sw, i_pu, i_po,
  i_ju, i_ca, i_cc, i_re, i_eq, i_ne, i_lt,
  i_gt, i_fe, i_st, i_ad, i_su, i_mu, i_di,
  i_an, i_or, i_xo, i_sh, i_zr, i_ha, i_ie,
  i_iq, i_ii
};

V process_opcode(NgaState *vm, CELL opcode) {
#ifdef FAST
  switch (opcode) {
    case 0: break;              case 1: i_li(vm); break;
    case 2: i_du(vm); break;    case 3: i_dr(vm); break;
    case 4: i_sw(vm); break;    case 5: i_pu(vm); break;
    case 6: i_po(vm); break;    case 7: i_ju(vm); break;
    case 8: i_ca(vm); break;    case 9: i_cc(vm); break;
    case 10: i_re(vm); break;   case 11: i_eq(vm); break;
    case 12: i_ne(vm); break;   case 13: i_lt(vm); break;
    case 14: i_gt(vm); break;   case 15: i_fe(vm); break;
    case 16: i_st(vm); break;   case 17: i_ad(vm); break;
    case 18: i_su(vm); break;   case 19: i_mu(vm); break;
    case 20: i_di(vm); break;   case 21: i_an(vm); break;
    case 22: i_or(vm); break;   case 23: i_xo(vm); break;
    case 24: i_sh(vm); break;   case 25: i_zr(vm); break;
    case 26: i_ha(vm); break;   case 27: i_ie(vm); break;
    case 28: i_iq(vm); break;   case 29: i_ii(vm); break;
    default: break;
  }
#else
  if (opcode != 0)
    instructions[opcode](vm);
#endif
}

#ifndef BRANCH_PREDICTION
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
#endif

V verbose_details(NgaState *vm, CELL opcode) {
  fprintf(stderr, "ip: %lld ", (long long)ACTIVE.ip);
  fprintf(stderr, "sp: %lld ", (long long)ACTIVE.sp);
  fprintf(stderr, "rp: %lld ", (long long)ACTIVE.rp);
  fprintf(stderr, "core: %lld ", (long long)vm->active);
  fprintf(stderr, "opcode: %lld\n", (long long)opcode);
}

#define INST(n) ((opcode >> n) & 0xFF) != 0
V process_opcode_bundle(NgaState *vm, CELL opcode) {
#ifndef BRANCH_PREDICTION
  if (INST(0))  instructions[opcode & 0xFF](vm);
  if (INST(8))  instructions[(opcode >> 8) & 0xFF](vm);
  if (INST(16)) instructions[(opcode >> 16) & 0xFF](vm);
  if (INST(24)) instructions[(opcode >> 24) & 0xFF](vm);
#else
  for (size_t i = 0; i < 4; ++i) {
    uint8_t current = ((uint8_t*)&opcode)[i];
    if (unlikely(current > 29)) {
      invalid_opcode(vm, opcode);
    }
    instructions[current](vm);
  }
#endif
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
  return(dlen + (src - osrc));  /* count does not include NUL */
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
      *dst = '\0';              /* NUL-terminate dst */
    while (*src++)
      ;
  }
  return(src - osrc - 1);       /* count does not include NUL */
}
#endif
