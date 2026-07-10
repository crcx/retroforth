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

#ifndef RETRO_NGA_CORE_H
#define RETRO_NGA_CORE_H

#define ACTIVE vm->cpu[vm->active]
#define TIB vm->memory[7]
#define TIB_END vm->memory[8]
#define TOS  ACTIVE.data[ACTIVE.sp]
#define NOS  ACTIVE.data[ACTIVE.sp-1]
#define TORS ACTIVE.address[ACTIVE.rp]

#define MAX_DEVICES      32
#define MAX_OPEN_FILES   32

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

/* VM core */
CELL stack_pop(NgaState *);
V stack_push(NgaState *, CELL);
V update_rx(NgaState *);
V register_device(NgaState *, V *, V *);
V load_embedded_image(NgaState *);
CELL load_image(NgaState *, char *);
V prepare_vm(NgaState *);
V execute(NgaState *, CELL);
V process_opcode_bundle(NgaState *, CELL);
#ifndef BRANCH_PREDICTION
V validate_opcode_bundle(NgaState *, CELL);
#endif
#ifdef ENABLE_MULTICORE
V switch_core(NgaState *);
#endif

/* Runtime hooks used by the VM core */
V handle_error(NgaState *, CELL);
V carry_out_abort(NgaState *);

/* String and scripting helpers shared by devices and the host runtime */
V display_utf8(const unsigned char *, int);
V utf32_to_utf8(uint32_t, unsigned char *, int *);
int read_character(int);
int fread_character(FILE *);
CELL string_inject(NgaState *, char *, CELL);
char *string_extract(NgaState *, CELL);
V include_file(NgaState *, char *, int);
V include_plain_file(NgaState *, char *, int);
V initialize_scripting(NgaState *);

#ifdef NEEDS_STRL
size_t strlcat(char *dst, const char *src, size_t dsize);
size_t strlcpy(char *dst, const char *src, size_t dsize);
#endif

#endif
