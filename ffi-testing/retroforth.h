/* Configuration ----------------------------------------------------- */
#include <unistd.h>
#include <stdio.h>

#ifdef ENABLE_MULTICORE
#define CORES 8
#else
#define CORES 1
#endif

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

#define TIB vm->memory[7]         /* Location of TIB                   */

#define MAX_DEVICES      32
#define MAX_OPEN_FILES   32

typedef struct NgaState NgaState;
typedef void (*Handler)(NgaState *);

struct NgaCore {
  CELL sp, rp, ip;              /* Stack & instruction pointers */
  CELL active;                  /* Is core active?              */
  CELL u;                       /* Should next operation be     */
                                /* unsigned?                    */
  CELL data[STACK_DEPTH];       /* The data stack               */
  CELL address[ADDRESSES];      /* The address stack            */

#ifdef ENABLE_MULTICORE
  CELL registers[24];           /* Internal Registers           */
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

  CELL Dictionary, NotFound, interpret;    /* Interfacing     */
  char string_data[8192];

#ifdef ENABLE_FLOATS
  double Floats[256], AFloats[256];        /* Floating Point */
  CELL fsp, afsp;
#endif

  /* Scripting */
  char **sys_argv;
  int sys_argc;
  char scripting_sources[64][8192];
  int current_source;
  int perform_abort;

  CELL currentLine;
  CELL ignoreToEOL, ignoreToEOF;

  /* Configuration of code & test fences for Unu */
  char code_start[256], code_end[256];
  char test_start[256], test_end[256];
  int codeBlocks;

  FILE *OpenFileHandles[MAX_OPEN_FILES];
};

/* Function Prototypes ----------------------------------------------- */
extern CELL stack_pop(NgaState *);
extern void stack_push(NgaState *, CELL);
extern CELL string_inject(NgaState *, char *, CELL);
extern char *string_extract(NgaState *, CELL);
extern void process_opcode_bundle(NgaState *, CELL);
extern void include_file(NgaState *, char *, int);
extern void register_device(NgaState *, void *, void *);
extern void execute(NgaState *, CELL);
