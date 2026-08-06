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

#include "nga_core.h"

#include "image.c"

#define IO(name) void io_name(NgaState *); void query_name(NgaState *);


/* Device Prototypes ------------------------------------------------- */
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

/* Global Variables -------------------------------------------------- */

int verbose;

#include "retro_modules.c"


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
  fwrite(vm->memory, sizeof(CELL), vm->memory[RETRO_IMAGE_HEAP] + 1, fp);
  fclose(fp);
}

V query_image(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_IMAGE);
}


/*=====================================================================*/


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

char *option_argument(int argc, char **argv, int *index) {
  if (*index + 1 >= argc) {
    fprintf(stderr, "ERROR: %s requires an argument\n", argv[*index]);
    exit(1);
  }
  return argv[++*index];
}

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
      include_file(vm, option_argument(argc, argv, &i), 0);
    } else if ARG("-p") {
      include_plain_file(vm, option_argument(argc, argv, &i), 0);
    } else if ARG("-u") {
      load_image(vm, option_argument(argc, argv, &i));
      update_rx(vm);
    } else if ARG("-r") {
      load_image(vm, option_argument(argc, argv, &i));
      modes[FLAG_INTERACTIVE] = 1;
      update_rx(vm);
    } else if ARG("-t") {
      include_file(vm, option_argument(argc, argv, &i), 1);
    } else if (ARG("--code-start") || ARG("-cs")) {
      strlcpy(vm->code_start, option_argument(argc, argv, &i), 256);
    } else if (ARG("--code-end") || ARG("-ce")) {
      strlcpy(vm->code_end, option_argument(argc, argv, &i), 256);
    } else if (ARG("--test-start") || ARG("-ts")) {
      strlcpy(vm->test_start, option_argument(argc, argv, &i), 256);
    } else if (ARG("--test-end") || ARG("-te")) {
      strlcpy(vm->test_end, option_argument(argc, argv, &i), 256);
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
