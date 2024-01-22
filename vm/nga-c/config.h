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

#ifndef MAKEFILE_CONFIG
#define ENABLE_FLOATS
#define ENABLE_FILES
#define ENABLE_UNIX
#define ENABLE_RNG
#define ENABLE_CLOCK
#define ENABLE_SCRIPTING
/* #define ENABLE_SOCKETS */
#define ENABLE_SIGNALS
#define ENABLE_MULTICORE
/* #define ENABLE_FFI */
#define ENABLE_ERROR
#define ENABLE_UNSIGNED
#define ENABLE_MALLOC
#define ENABLE_BLOCKS
#define ENABLE_IOCTL
#endif

#ifdef ENABLE_MULTICORE
#define CORES 8
#else
#define CORES 1
#endif

#if defined(_WIN32) || defined(_WIN64)
#define NEEDS_STRL
#endif

#if defined(__APPLE__) && defined(__MACH__) && defined(NEEDS_STRL)
#undef NEEDS_STRL
#endif

/* Configuration ----------------------------------------------------- */
#ifndef BIT64
#define CELL int32_t
#define CELL_MIN INT_MIN + 1
#define CELL_MAX INT_MAX - 1
#else
#define CELL int64_t
#define CELL_MIN LLONG_MIN + 1
#define CELL_MAX LLONG_MAX - 1
#endif

#ifndef IMAGE_SIZE
#define IMAGE_SIZE   524288       /* Amount of RAM, in cells */
#endif

#ifndef ADDRESSES
#define ADDRESSES    256          /* Depth of address stack */
#endif

#ifndef STACK_DEPTH
#define STACK_DEPTH  256          /* Depth of data stack */
#endif

#ifdef BRANCH_PREDICTION
/* The Compiler Magic Trick */
#define unlikely(x) __builtin_expect((x),0)
#endif
