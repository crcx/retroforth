/*---------------------------------------------------------------------
  Copyright (c) 2008 - 2022, Charles Childers

  Portions are based on Ngaro, which was additionally copyright
  by the following:

  Copyright (c) 2009 - 2010, Luke Parrish
  Copyright (c) 2010,        Marc Simpson
  Copyright (c) 2010,        Jay Skeer
  Copyright (c) 2011,        Kenneth Keating
  ---------------------------------------------------------------------*/

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

#ifdef ENABLE_SIGNALS
#include <signal.h>
#endif

#ifdef ENABLE_FLOATS
#include <math.h>
#endif

#ifdef ENABLE_SOCKETS
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#ifdef ENABLE_UNIX
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifdef ENABLE_FFI
#include <dlfcn.h>
#endif

#ifdef ENABLE_MULTICORE
#define CORES 8
#else
#define CORES 1
#endif

#ifdef _WIN32
#define NEEDS_STRL
#endif

#ifdef _WIN64
#define NEEDS_STRL
#endif

#if defined(__APPLE__) && defined(__MACH__)
#ifdef NEEDS_STRL
#undef NEEDS_STRL
#endif
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

#define TIB vm->memory[7]         /* Location of TIB                   */

#define MAX_DEVICES      32
#define MAX_OPEN_FILES   32

#include "image.c"

typedef struct NgaState NgaState;

typedef void (*Handler)(NgaState *);

struct NgaCore {
  CELL sp, rp, ip, active, u;     /* Stack & instruction pointers      */
  CELL data[STACK_DEPTH];         /* The data stack                    */
  CELL address[ADDRESSES];        /* The address stack                 */
#ifdef ENABLE_MULTICORE
  CELL registers[24];
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

  /* Interfacing */
  CELL Dictionary, NotFound, interpret;
  char string_data[8192];

  /* Floating Point */
  double Floats[256], AFloats[256];
  CELL fsp, afsp;

  /* Scripting */
  char **sys_argv;
  int sys_argc;
  char scripting_sources[64][8192];
  int current_source;
  int perform_abort;

  CELL currentLine;
  CELL ignoreToEOL;
  CELL ignoreToEOF;

  char code_start[256], code_end[256];
  char test_start[256], test_end[256];
  int codeBlocks;
};

/* Function Prototypes ----------------------------------------------- */
CELL stack_pop(NgaState *);
void stack_push(NgaState *, CELL);
CELL string_inject(NgaState *, char *, CELL);
char *string_extract(NgaState *, CELL);
void update_rx(NgaState *);
void include_file(NgaState *, char *, int);

void register_device(NgaState *, void *, void *);

void io_output(NgaState *);           void query_output(NgaState *);
void io_keyboard(NgaState *);         void query_keyboard(NgaState *);
void query_filesystem(NgaState *);    void io_filesystem(NgaState *);
void io_clock(NgaState *);            void query_clock(NgaState *);
void io_scripting(NgaState *);        void query_scripting(NgaState *);
void io_rng(NgaState *);              void query_rng(NgaState *);
void io_unsigned(NgaState *);         void query_unsigned(NgaState *);

#ifdef ENABLE_UNIX
void query_unix(NgaState *);          void io_unix(NgaState *);
#endif

#ifdef ENABLE_FLOATS
void io_floatingpoint(NgaState *);    void query_floatingpoint(NgaState *);
#endif

#ifdef ENABLE_SOCKETS
void io_socket(NgaState *);           void query_socket(NgaState *);
#endif

void io_image(NgaState *);            void query_image(NgaState *);

void load_embedded_image(NgaState *);
CELL load_image(NgaState *, char *);
void prepare_vm(NgaState *);
void process_opcode_bundle(NgaState *, CELL);
int validate_opcode_bundle(CELL);

#ifdef NEEDS_STRL
size_t strlcat(char *dst, const char *src, size_t dsize);
size_t strlcpy(char *dst, const char *src, size_t dsize);
#endif

void prepare_vm();

void inst_no(NgaState *);  void inst_li(NgaState *);  void inst_du(NgaState *);
void inst_dr(NgaState *);  void inst_sw(NgaState *);  void inst_pu(NgaState *);
void inst_po(NgaState *);  void inst_ju(NgaState *);  void inst_ca(NgaState *);
void inst_cc(NgaState *);  void inst_re(NgaState *);  void inst_eq(NgaState *);
void inst_ne(NgaState *);  void inst_lt(NgaState *);  void inst_gt(NgaState *);
void inst_fe(NgaState *);  void inst_st(NgaState *);  void inst_ad(NgaState *);
void inst_su(NgaState *);  void inst_mu(NgaState *);  void inst_di(NgaState *);
void inst_an(NgaState *);  void inst_or(NgaState *);  void inst_xo(NgaState *);
void inst_sh(NgaState *);  void inst_zr(NgaState *);  void inst_ha(NgaState *);
void inst_ie(NgaState *);  void inst_iq(NgaState *);  void inst_ii(NgaState *);


/* Image, Stack, and VM variables ------------------------------------ */
#define TOS  vm->cpu[vm->active].data[vm->cpu[vm->active].sp]
#define NOS  vm->cpu[vm->active].data[vm->cpu[vm->active].sp-1]
#define TORS vm->cpu[vm->active].address[vm->cpu[vm->active].rp]

/* Global Variables -------------------------------------------------- */

/* Multi Core Support ------------------------------------------------ */
#ifdef ENABLE_MULTICORE
void init_core(NgaState *vm, CELL x) {
  int y;
  vm->cpu[x].sp = 0;
  vm->cpu[x].rp = 0;
  vm->cpu[x].ip = 0;
  vm->cpu[x].active = 0;
  vm->cpu[x].u = 0;
  for (y = 0; y < STACK_DEPTH; y++) { vm->cpu[x].data[y] = 0; };
  for (y = 0; y < ADDRESSES; y++) { vm->cpu[x].address[y] = 0; };
  for (y = 0; y < 24; y++) { vm->cpu[x].registers[y] = 0; };
}

void start_core(NgaState *vm, CELL x, CELL ip) {
  vm->cpu[x].ip = ip;
  vm->cpu[x].rp = 1;
  vm->cpu[x].active = -1;
}

void pause_core(NgaState *vm, CELL x) {
  vm->cpu[x].active = 0;
}

void resume_core(NgaState *vm, CELL x) {
  vm->cpu[x].active = -1;
}

void switch_core(NgaState *vm) {
  vm->active += 1;
  if (vm->active >= CORES) { vm->active = 0; }
  if (!vm->cpu[vm->active].active) { switch_core(vm); }
}

void io_multicore(NgaState *vm) {
  int x, y, z;
  x = stack_pop(vm);
  switch(x) {
    case 0: y = stack_pop(vm);
            init_core(vm, y);
            break;
    case 1: y = stack_pop(vm);
            z = stack_pop(vm);
            start_core(vm, y, z);
            break;
    case 2: y = stack_pop(vm);
            pause_core(vm, y);
            break;
    case 3: pause_core(vm, vm->active);
            break;
    case 4: y = stack_pop(vm);
            resume_core(vm, y);
            break;
    case 5: y = stack_pop(vm);
            stack_push(vm, vm->cpu[vm->active].registers[y]);
            break;
    case 6: y = stack_pop(vm);
            z = stack_pop(vm);
            vm->cpu[vm->active].registers[y] = z;
            break;
  }
}

void query_multicore(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 8000);  /* device type 8000 */
}
#endif



/* External Functions ------------------------------------------------ */

#ifdef ENABLE_FFI
typedef void (*External)(void *, void *, void *);

void *handles[32];
External funcs[32000];
int nlibs, nffi;

void open_library(NgaState *vm) {
  handles[nlibs] = dlopen(string_extract(vm, stack_pop(vm)), RTLD_LAZY);
  stack_push(vm, nlibs);
  nlibs++;
}

void map_symbol(NgaState *vm) {
  int h;
  h = stack_pop(vm);
  char *s = string_extract(vm, stack_pop(vm));
  funcs[nffi] = dlsym(handles[h], s);
  stack_push(vm, nffi);
  nffi++;
}

void invoke(NgaState *vm) {
  funcs[stack_pop(vm)](stack_push, stack_pop, vm->memory);
}

void io_ffi(NgaState *vm) {
  switch (stack_pop(vm)) {
    case 0: open_library(vm); break;
    case 1: map_symbol(vm); break;
    case 2: invoke(vm); break;
  }
}

void query_ffi(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 8100);  /* device type 8100 */
}
#endif



/* Floating Point ---------------------------------------------------- */

#ifdef ENABLE_FLOATS
void float_guard(NgaState *vm) {
  if (vm->fsp < 0 || vm->fsp > 255) {
    printf("\nERROR (nga/float_guard): Float Stack Limits Exceeded!\n");
    printf("At %lld, fsp = %lld\n", (long long)vm->cpu[vm->active].ip, (long long)vm->fsp);
    exit(1);
  }
  if (vm->afsp < 0 || vm->afsp > 255) {
    printf("\nERROR (nga/float_guard): Alternate Float Stack Limits Exceeded!\n");
    printf("At %lld, afsp = %lld\n", (long long)vm->cpu[vm->active].ip, (long long)vm->afsp);
    exit(1);
  }
}

/*---------------------------------------------------------------------
  The first two functions push a float to the stack and pop a value off
  the stack.
  ---------------------------------------------------------------------*/

void float_push(NgaState *vm, double value) {
  vm->fsp++;
  float_guard(vm);
  vm->Floats[vm->fsp] = value;
}

double float_pop(NgaState *vm) {
  vm->fsp--;
  float_guard(vm);
  return vm->Floats[vm->fsp + 1];
}

void float_to_alt(NgaState *vm) {
  vm->afsp++;
  float_guard(vm);
  vm->AFloats[vm->afsp] = float_pop(vm);
}

void float_from_alt(NgaState *vm) {
  float_push(vm, vm->AFloats[vm->afsp]);
  vm->afsp--;
  float_guard(vm);
}


/*---------------------------------------------------------------------
  RETRO operates on 32-bit signed integer values. This function just
  pops a number from the data stack, casts it to a float, and pushes it
  to the float stack.
  ---------------------------------------------------------------------*/
void float_from_number(NgaState *vm) {
    float_push(vm, (double)stack_pop(vm));
}


/*---------------------------------------------------------------------
  To get a float from a string in the image, I provide this function.
  I cheat: using `atof()` takes care of the details, so I don't have
  to.
  ---------------------------------------------------------------------*/
void float_from_string(NgaState *vm) {
    float_push(vm, atof(string_extract(vm, stack_pop(vm))));
}


/*---------------------------------------------------------------------
  Converting a floating point into a string is slightly more work. Here
  I pass it off to `snprintf()` to deal with.
  ---------------------------------------------------------------------*/
void float_to_string(NgaState *vm) {
    snprintf(vm->string_data, 8192, "%f", float_pop(vm));
    string_inject(vm, vm->string_data, stack_pop(vm));
}


/*---------------------------------------------------------------------
  Converting a floating point back into a standard number requires a
  little care due to the signed nature. This makes adjustments for the
  max & min value, and then casts (rounding) the float back to a normal
  number.
  ---------------------------------------------------------------------*/

void float_to_number(NgaState *vm) {
  double a = float_pop(vm);
  if (a > 2147483647)
    a = 2147483647;
  if (a < -2147483648)
    a = -2147483648;
  stack_push(vm, (CELL)round(a));
}

void float_add(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, a+b);
}

void float_sub(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, b-a);
}

void float_mul(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, a*b);
}

void float_div(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, b/a);
}

void float_floor(NgaState *vm) {
  float_push(vm, floor(float_pop(vm)));
}

void float_ceil(NgaState *vm) {
  float_push(vm, ceil(float_pop(vm)));
}

void float_eq(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  if (a == b)
    stack_push(vm, -1);
  else
    stack_push(vm, 0);
}

void float_neq(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  if (a != b)
    stack_push(vm, -1);
  else
    stack_push(vm, 0);
}

void float_lt(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  if (b < a)
    stack_push(vm, -1);
  else
    stack_push(vm, 0);
}

void float_gt(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  if (b > a)
    stack_push(vm, -1);
  else
    stack_push(vm, 0);
}

void float_depth(NgaState *vm) {
  stack_push(vm, vm->fsp);
}

void float_adepth(NgaState *vm) {
  stack_push(vm, vm->afsp);
}

void float_dup(NgaState *vm) {
  double a = float_pop(vm);
  float_push(vm, a);
  float_push(vm, a);
}

void float_drop(NgaState *vm) {
  float_pop(vm);
}

void float_swap(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, a);
  float_push(vm, b);
}

void float_log(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, log(b) / log(a));
}

void float_sqrt(NgaState *vm) {
  float_push(vm, sqrt(float_pop(vm)));
}

void float_pow(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, pow(b, a));
}

void float_sin(NgaState *vm) {
  float_push(vm, sin(float_pop(vm)));
}

void float_cos(NgaState *vm) {
  float_push(vm, cos(float_pop(vm)));
}

void float_tan(NgaState *vm) {
  float_push(vm, tan(float_pop(vm)));
}

void float_asin(NgaState *vm) {
  float_push(vm, asin(float_pop(vm)));
}

void float_acos(NgaState *vm) {
  float_push(vm, acos(float_pop(vm)));
}

void float_atan(NgaState *vm) {
  float_push(vm, atan(float_pop(vm)));
}


/*---------------------------------------------------------------------
  With this finally done, I implement the FPU instructions.
  ---------------------------------------------------------------------*/
Handler FloatHandlers[] = {
  float_from_number,  float_from_string,
  float_to_number,    float_to_string,
  float_add,      float_sub,     float_mul,   float_div,
  float_floor,    float_ceil,    float_sqrt,  float_eq,
  float_neq,      float_lt,      float_gt,    float_depth,
  float_dup,      float_drop,    float_swap,  float_log,
  float_pow,      float_sin,     float_tan,   float_cos,
  float_asin,     float_acos,    float_atan,  float_to_alt,
  float_from_alt, float_adepth,
};

void query_floatingpoint(NgaState *vm) {
  stack_push(vm, 1);
  stack_push(vm, 2);
}

void io_floatingpoint(NgaState *vm) {
  FloatHandlers[stack_pop(vm)](vm);
}
#endif


/* FileSystem Device ------------------------------------------------- */

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

void file_open(NgaState *vm) {
  CELL slot, mode, name;
  char *request;
  slot = files_get_handle();
  mode = stack_pop(vm);
  name = stack_pop(vm);
  request = string_extract(vm, name);
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
  stack_push(vm, slot);
}


/*---------------------------------------------------------------------
  `file_read()` reads a byte from a file. This takes a file pointer
  from the stack and pushes the character that was read to the stack.
  ---------------------------------------------------------------------*/

void file_read(NgaState *vm) {
  CELL c;
  CELL slot = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_read): Invalid file handle\n");
    exit(1);
  }
  c = fgetc(OpenFileHandles[slot]);
  stack_push(vm, feof(OpenFileHandles[slot]) ? 0 : c);
}


/*---------------------------------------------------------------------
  `file_write()` writes a byte to a file. This takes a file pointer
  (TOS) and a byte (NOS) from the stack. It does not return any values
  on the stack.
  ---------------------------------------------------------------------*/

void file_write(NgaState *vm) {
  CELL slot, c, r;
  slot = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_write): Invalid file handle\n");
    exit(1);
  }
  c = stack_pop(vm);
  r = fputc(c, OpenFileHandles[slot]);
}


/*---------------------------------------------------------------------
  `file_close()` closes a file. This takes a file handle from the
  stack and does not return anything on the stack.
  ---------------------------------------------------------------------*/

void file_close(NgaState *vm) {
  CELL slot = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_close): Invalid file handle\n");
    exit(1);
  }
  fclose(OpenFileHandles[slot]);
  OpenFileHandles[slot] = 0;
}


/*---------------------------------------------------------------------
  `file_get_position()` provides the current index into a file. This
  takes the file handle from the stack and returns the offset.
  ---------------------------------------------------------------------*/

void file_get_position(NgaState *vm) {
  CELL slot = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_get_position): Invalid file handle\n");
    exit(1);
  }
  stack_push(vm, (CELL) ftell(OpenFileHandles[slot]));
}


/*---------------------------------------------------------------------
  `file_set_position()` changes the current index into a file to the
  specified one. This takes a file handle (TOS) and new offset (NOS)
  from the stack.
  ---------------------------------------------------------------------*/

void file_set_position(NgaState *vm) {
  CELL slot, pos;
  slot = stack_pop(vm);
  pos  = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_set_position): Invalid file handle\n");
    exit(1);
  }
  fseek(OpenFileHandles[slot], pos, SEEK_SET);
}


/*---------------------------------------------------------------------
  `file_get_size()` returns the size of a file, or 0 if empty. If the
  file is a directory, it returns -1. It takes a file handle from the
  stack.
  ---------------------------------------------------------------------*/

void file_get_size(NgaState *vm) {
  CELL slot, current, r, size;
  struct stat buffer;
  slot = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_get_size): Invalid file handle\n");
    exit(1);
  }
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
  stack_push(vm, (r == 0) ? size : 0);
}


/*---------------------------------------------------------------------
  `file_delete()` removes a file. This takes a file name (as a string)
  from the stack.
  ---------------------------------------------------------------------*/

void file_delete(NgaState *vm) {
  char *request;
  CELL name = stack_pop(vm);
  request = string_extract(vm, name);
  unlink(request);
}


/*---------------------------------------------------------------------
  `file_flush()` flushes any pending writes to disk. This takes a
  file handle from the stack.
  ---------------------------------------------------------------------*/

void file_flush(NgaState *vm) {
  CELL slot;
  slot = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_flush): Invalid file handle\n");
    exit(1);
  }
  fflush(OpenFileHandles[slot]);
}

Handler FileActions[10] = {
  file_open,          file_close,
  file_read,          file_write,
  file_get_position,  file_set_position,
  file_get_size,      file_delete,
  file_flush
};

void query_filesystem(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 4);
}

void io_filesystem(NgaState *vm) {
  FileActions[stack_pop(vm)](vm);
}


#ifdef ENABLE_UNIX
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

void unix_open_pipe(NgaState *vm) {
  CELL slot, mode, name;
  char *request;
  slot = files_get_handle();
  mode = stack_pop(vm);
  name = stack_pop(vm);
  request = string_extract(vm, name);
  if (slot > 0) {
    if (mode == 0)  OpenFileHandles[slot] = popen(request, "r");
    if (mode == 1)  OpenFileHandles[slot] = popen(request, "w");
    if (mode == 3)  OpenFileHandles[slot] = popen(request, "r+");
  }
  if (OpenFileHandles[slot] == NULL) {
    OpenFileHandles[slot] = 0;
    slot = 0;
  }
  stack_push(vm, slot);
}

void unix_close_pipe(NgaState *vm) {
  pclose(OpenFileHandles[TOS]);
  OpenFileHandles[TOS] = 0;
  stack_pop(vm);
}

void unix_system(NgaState *vm) {
  int ignore = 0;
  ignore = system(string_extract(vm, stack_pop(vm)));
}

void unix_fork(NgaState *vm) {
  stack_push(vm, fork());
}

void unix_run_external(NgaState *vm) {
  char *line, *args[128];
  int i, status;
  pid_t pid;

  char **argv = args;
  line = string_extract(vm, stack_pop(vm));

  for(i = 0; i < 128; i++)
    args[i] = 0;

  while (*line != '\0') {
    while (*line == ' ' || *line == '\t' || *line == '\n')
      *line++ = '\0';
    *argv++ = line;
    while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n')
      line++;
  }

  if ((pid = fork()) < 0) {
    printf("*** ERROR: forking child process failed\n");
    exit(1);
  }
  else if (pid == 0) {
    int e = execvp(*args, args);
    if (e < 0) {
      printf("*** ERROR: exec failed with %d\n", e);
      exit(1);
    }
  } else {
  while (wait(&status) != pid)
    ;
  }
}


/*---------------------------------------------------------------------
  UNIX provides `execl` to execute a file, with various forms for
  arguments provided.

  RRE wraps this in several functions, one for each number of passed
  arguments. See the Glossary for details on what each takes from the
  stack. Each of these will return the error code if the execution
  fails.
  ---------------------------------------------------------------------*/

void unix_exec0(NgaState *vm) {
  char path[1025];
  strlcpy(path, string_extract(vm, stack_pop(vm)), 1024);
  execl(path, path, (char *)0);
  stack_push(vm, errno);
}

void unix_exec1(NgaState *vm) {
  char path[1025];
  char arg0[1025];
  strlcpy(arg0, string_extract(vm, stack_pop(vm)), 1024);
  strlcpy(path, string_extract(vm, stack_pop(vm)), 1024);
  execl(path, path, arg0, (char *)0);
  stack_push(vm, errno);
}

void unix_exec2(NgaState *vm) {
  char path[1025];
  char arg0[1025], arg1[1025];
  strlcpy(arg1, string_extract(vm, stack_pop(vm)), 1024);
  strlcpy(arg0, string_extract(vm, stack_pop(vm)), 1024);
  strlcpy(path, string_extract(vm, stack_pop(vm)), 1024);
  execl(path, path, arg0, arg1, (char *)0);
  stack_push(vm, errno);
}

void unix_exec3(NgaState *vm) {
  char path[1025];
  char arg0[1025], arg1[1025], arg2[1025];
  strlcpy(arg2, string_extract(vm, stack_pop(vm)), 1024);
  strlcpy(arg1, string_extract(vm, stack_pop(vm)), 1024);
  strlcpy(arg0, string_extract(vm, stack_pop(vm)), 1024);
  strlcpy(path, string_extract(vm, stack_pop(vm)), 1024);
  execl(path, path, arg0, arg1, arg2, (char *)0);
  stack_push(vm, errno);
}

void unix_exit(NgaState *vm) {
  exit(stack_pop(vm));
}

void unix_getpid(NgaState *vm) {
  stack_push(vm, getpid());
}

void unix_wait(NgaState *vm) {
  int a;
  stack_push(vm, wait(&a));
}

void unix_kill(NgaState *vm) {
  CELL a;
  a = stack_pop(vm);
  kill(stack_pop(vm), a);
}

void unix_write(NgaState *vm) {
  CELL a, b, c;
  ssize_t ignore;
  c = stack_pop(vm);
  b = stack_pop(vm);
  a = stack_pop(vm);
  ignore = write(fileno(OpenFileHandles[c]), string_extract(vm, a), b);
}

void unix_chdir(NgaState *vm) {
  int ignore;
  ignore = chdir(string_extract(vm, stack_pop(vm)));
}

void unix_getenv(NgaState *vm) {
  CELL a, b;
  a = stack_pop(vm);
  b = stack_pop(vm);
  string_inject(vm, getenv(string_extract(vm, b)), a);
}

void unix_putenv(NgaState *vm) {
  putenv(string_extract(vm, stack_pop(vm)));
}

void unix_sleep(NgaState *vm) {
  sleep(stack_pop(vm));
}

Handler UnixActions[] = {
  unix_system,    unix_fork,       unix_exec0,   unix_exec1,   unix_exec2,
  unix_exec3,     unix_exit,       unix_getpid,  unix_wait,    unix_kill,
  unix_open_pipe, unix_close_pipe, unix_write,   unix_chdir,   unix_getenv,
  unix_putenv,    unix_sleep,      unix_run_external
};

void query_unix(NgaState *vm) {
  stack_push(vm, 3);
  stack_push(vm, 8);
}

void io_unix(NgaState *vm) {
  UnixActions[stack_pop(vm)](vm);
}
#endif


/* Time and Date Functions --------------------------------------------*/
#ifdef ENABLE_CLOCK
void clock_time(NgaState *vm) {
  stack_push(vm, (CELL)time(NULL));
}

void clock_day(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)localtime(&t)->tm_mday);
}

void clock_month(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)localtime(&t)->tm_mon + 1);
}

void clock_year(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)localtime(&t)->tm_year + 1900);
}

void clock_hour(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)localtime(&t)->tm_hour);
}

void clock_minute(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)localtime(&t)->tm_min);
}

void clock_second(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)localtime(&t)->tm_sec);
}

void clock_day_utc(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)gmtime(&t)->tm_mday);
}

void clock_month_utc(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)gmtime(&t)->tm_mon + 1);
}

void clock_year_utc(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)gmtime(&t)->tm_year + 1900);
}

void clock_hour_utc(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)gmtime(&t)->tm_hour);
}

void clock_minute_utc(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)gmtime(&t)->tm_min);
}

void clock_second_utc(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)gmtime(&t)->tm_sec);
}

Handler ClockActions[] = {
  clock_time,
  clock_day,      clock_month,      clock_year,
  clock_hour,     clock_minute,     clock_second,
  clock_day_utc,  clock_month_utc,  clock_year_utc,
  clock_hour_utc, clock_minute_utc, clock_second_utc
};

void query_clock(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 5);
}

void io_clock(NgaState *vm) {
  ClockActions[stack_pop(vm)](vm);
}
#endif


/* Random Number Generator --------------------------------------------*/
#ifdef ENABLE_RNG
void io_rng(NgaState *vm) {
  int64_t r = 0;
  char buffer[8];
  int i;
  ssize_t ignore;
  int fd = open("/dev/urandom", O_RDONLY);
  ignore = read(fd, buffer, 8);
  close(fd);
  for(i = 0; i < 8; ++i) {
    r = r << 8;
    r += ((int64_t)buffer[i] & 0xFF);
  }
#ifndef BIT64
  stack_push(vm, (CELL)abs((CELL)r));
#else
  stack_push(vm, (CELL)llabs((CELL)r));
#endif
}

void query_rng(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 10);
}
#endif


#ifdef ENABLE_SOCKETS
/*---------------------------------------------------------------------
  BSD Sockets
  ---------------------------------------------------------------------*/

int SocketID[16];
struct sockaddr_in Sockets[16];

struct addrinfo hints, *res;

void socket_getaddrinfo(NgaState *vm) {
  char host[1025], port[6];
  strlcpy(port, string_extract(vm, stack_pop(vm)), 5);
  strlcpy(host, string_extract(vm, stack_pop(vm)), 1024);
  getaddrinfo(host, port, &hints, &res);
}

void socket_get_host(NgaState *vm) {
  struct hostent *hp;
  struct in_addr **addr_list;

  hp = gethostbyname(string_extract(vm, stack_pop(vm)));
  if (hp == NULL) {
    vm->memory[stack_pop(vm)] = 0;
    return;
  }

  addr_list = (struct in_addr **)hp->h_addr_list;
  string_inject(vm, inet_ntoa(*addr_list[0]), stack_pop(vm));
}

void socket_create(NgaState *vm) {
  int i;
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  for (i = 0; i < 16; i++) {
    if (SocketID[i] == 0 && sock != 0) {
      SocketID[i] = sock;
      stack_push(vm, (CELL)i);
      sock = 0;
    }
  }
}

void socket_bind(NgaState *vm) {
  int sock, port;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  sock = stack_pop(vm);
  port = stack_pop(vm);

  getaddrinfo(NULL, string_extract(vm, port), &hints, &res);
  stack_push(vm, (CELL) bind(SocketID[sock], res->ai_addr, res->ai_addrlen));
  stack_push(vm, errno);
}

void socket_listen(NgaState *vm) {
  int sock = stack_pop(vm);
  int backlog = stack_pop(vm);
  stack_push(vm, listen(SocketID[sock], backlog));
  stack_push(vm, errno);
}

void socket_accept(NgaState *vm) {
  int i;
  int sock = stack_pop(vm);
  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;
  int new_fd = accept(SocketID[sock], (struct sockaddr *)&their_addr, &addr_size);

  for (i = 0; i < 16; i++) {
    if (SocketID[i] == 0 && new_fd != 0) {
      SocketID[i] = new_fd;
      stack_push(vm, (CELL)i);
      new_fd = 0;
    }
  }
  stack_push(vm, errno);
}

void socket_connect(NgaState *vm) {
  stack_push(vm, (CELL)connect(SocketID[stack_pop(vm)], res->ai_addr, res->ai_addrlen));
  stack_push(vm, errno);
}

void socket_send(NgaState *vm) {
  int sock = stack_pop(vm);
  char *buf = string_extract(vm, stack_pop(vm));
  stack_push(vm, send(SocketID[sock], buf, strlen(buf), 0));
  stack_push(vm, errno);
}

void socket_sendto(NgaState *vm) {
}

void socket_recv(NgaState *vm) {
  char buf[8193];
  int sock = stack_pop(vm);
  int limit = stack_pop(vm);
  int dest = stack_pop(vm);
  int len = recv(SocketID[sock], buf, limit, 0);
  if (len > 0)  buf[len] = '\0';
  if (len > 0)  string_inject(vm, buf, dest);
  stack_push(vm, len);
  stack_push(vm, errno);
}

void socket_recvfrom(NgaState *vm) {
}

void socket_close(NgaState *vm) {
  int sock = stack_pop(vm);
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

void io_socket(NgaState *vm) {
  SocketActions[stack_pop(vm)](vm);
}

void query_socket(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 7);
}
#endif


/*---------------------------------------------------------------------
  Now on to I/O and extensions!
  ---------------------------------------------------------------------*/

void io_output(NgaState *vm) {
  putc(stack_pop(vm), stdout);
  fflush(stdout);
}

void query_output(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 0);
}


/*=====================================================================*/

void io_keyboard(NgaState *vm) {
  stack_push(vm, getc(stdin));
  if (TOS == 127) TOS = 8;
}

void query_keyboard(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 1);
}

/*=====================================================================*/

#ifdef ENABLE_UNSIGNED
void io_unsigned(NgaState *vm) {
  int x, y, z;
  long c;
  switch (stack_pop(vm)) {
    case 0: vm->cpu[vm->active].u = 1; break;
    case 1:
      c = 0;
      z = stack_pop(vm);
      y = stack_pop(vm);
      x = stack_pop(vm);
      if (vm->cpu[vm->active].u != 0) {
        c = (unsigned)x * (unsigned)y;
        stack_push(vm, (unsigned)c % (unsigned)z);
        stack_push(vm, (unsigned)c / (unsigned)z);
      }
      else {
        c = x * y;
        stack_push(vm, c % z);
        stack_push(vm, c / z);
      }
      vm->cpu[vm->active].u = 0;
      break;
  }
}

void query_unsigned(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 8101);
}
#endif

/*=====================================================================*/

void io_image(NgaState *vm) {
  FILE *fp;
  char *f = string_extract(vm, stack_pop(vm));
  if ((fp = fopen(f, "wb")) == NULL) {
    printf("\nERROR (nga/io_image): Unable to save the image: %s!\n", f);
    exit(2);
  }
  fwrite(vm->memory, sizeof(CELL), vm->memory[3] + 1, fp);
  fclose(fp);
}

void query_image(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 1000);
}


/*=====================================================================*/


/*---------------------------------------------------------------------
  Scripting Support
  ---------------------------------------------------------------------*/

void scripting_arg(NgaState *vm) {
  CELL a, b;
  a = stack_pop(vm);
  b = stack_pop(vm);
  stack_push(vm, string_inject(vm, vm->sys_argv[a + 2], b));
}

void scripting_arg_count(NgaState *vm) {
  stack_push(vm, vm->sys_argc - 2);
}

void scripting_include(NgaState *vm) {
  include_file(vm, string_extract(vm, stack_pop(vm)), 0);
}

void scripting_name(NgaState *vm) {
  stack_push(vm, string_inject(vm, vm->sys_argv[1], stack_pop(vm)));
}

/* addeded in scripting i/o device, revision 1 */
void scripting_source(NgaState *vm) {
  stack_push(vm, string_inject(vm, vm->scripting_sources[vm->current_source], stack_pop(vm)));
}

void scripting_line(NgaState *vm) {
  stack_push(vm, vm->currentLine);
}

void scripting_ignore_to_eol(NgaState *vm) {
  vm->ignoreToEOL = -1;
}

void scripting_ignore_to_eof(NgaState *vm) {
  vm->ignoreToEOF = -1;
}

void scripting_abort(NgaState *vm) {
  scripting_ignore_to_eol(vm);
  scripting_ignore_to_eof(vm);
  vm->perform_abort = -1;
}

void carry_out_abort(NgaState *vm) {
  vm->cpu[vm->active].ip = IMAGE_SIZE + 1;
  vm->cpu[vm->active].rp = 0;
  vm->cpu[vm->active].sp = 0;
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

Handler ScriptingActions[] = {
  scripting_arg_count,     scripting_arg,
  scripting_include,       scripting_name,
  scripting_source,        scripting_line,
  scripting_ignore_to_eol, scripting_ignore_to_eof,
  scripting_abort
};

void query_scripting(NgaState *vm) {
  stack_push(vm, 2);
  stack_push(vm, 9);
}

void io_scripting(NgaState *vm) {
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

void invalid_opcode(NgaState *vm, CELL opcode) {
  CELL a, i;
  printf("\nERROR (nga/execute): Invalid instruction!\n");
  printf("At %lld, opcode %lld\n", (long long)vm->cpu[vm->active].ip, (long long)opcode);
  printf("Instructions: ");
  a = opcode;
  for (i = 0; i < 4; i++) {
    printf("%lldd ", (long long)a & 0xFF);
    a = a >> 8;
  }
  printf("\n");
  exit(1);
}

void execute(NgaState *vm, CELL cell) {
  CELL token;
  CELL opcode;
  if (vm->cpu[vm->active].rp == 0)
    vm->cpu[vm->active].rp = 1;
  vm->cpu[vm->active].ip = cell;
  token = TIB;
  while (vm->cpu[vm->active].ip < IMAGE_SIZE) {
    if (vm->perform_abort == 0) {
      if (vm->cpu[vm->active].ip == vm->NotFound) {
        printf("\nERROR: Word Not Found: ");
        printf("`%s`\n\n", string_extract(vm, token));
      }
      if (vm->cpu[vm->active].ip == vm->interpret) {
        token = TOS;
      }
      opcode = vm->memory[vm->cpu[vm->active].ip];
      if (validate_opcode_bundle(opcode) != 0) {
        process_opcode_bundle(vm, opcode);
      } else {
        invalid_opcode(vm, opcode);
      }
      if (vm->cpu[vm->active].sp < 0 || vm->cpu[vm->active].sp > STACK_DEPTH) {
        printf("\nERROR (nga/execute): Stack Limits Exceeded!\n");
        printf("At %lld, opcode %lld. sp = %lld, core = %lld\n", (long long)vm->cpu[vm->active].ip, (long long)opcode, (long long)vm->cpu[vm->active].sp, (long long)vm->active);
        exit(1);
      }
      if (vm->cpu[vm->active].rp < 0 || vm->cpu[vm->active].rp > ADDRESSES) {
        printf("\nERROR (nga/execute): Address Stack Limits Exceeded!\n");
        printf("At %lld, opcode %lld. rp = %lld\n", (long long)vm->cpu[vm->active].ip, (long long)opcode, (long long)vm->cpu[vm->active].rp);
        exit(1);
      }
      vm->cpu[vm->active].ip++;
#ifdef ENABLE_MULTICORE
      switch_core(vm);
#endif
      if (vm->cpu[vm->active].rp == 0)
        vm->cpu[vm->active].ip = IMAGE_SIZE;
    } else {
      carry_out_abort(vm);
    }
  }
}


/*---------------------------------------------------------------------
  RETRO's `interpret` word expects a token on the stack. This next
  function copies a token to the `TIB` (text input buffer) and then
  calls `interpret` to process it.
  ---------------------------------------------------------------------*/

void evaluate(NgaState *vm, char *s) {
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

void read_token(FILE *file, char *token_buffer) {
  int ch = getc(file);
  int count = 0;
  while (not_eol(ch)) {
    if ((ch == 8 || ch == 127) && count > 0) {
      count--;
    } else {
      token_buffer[count++] = ch;
    }
    ch = getc(file);
  }
  token_buffer[count] = '\0';
}


void skip_indent(FILE *fp) {
  int ch = getc(fp);
  while (ch == ' ') {
    ch = getc(fp);
  }
  ungetc(ch, fp);
}

/*---------------------------------------------------------------------
  Display the Stack Contents
  ---------------------------------------------------------------------*/

void dump_stack(NgaState *vm) {
  CELL i;
  if (vm->cpu[vm->active].sp == 0)  return;
  printf("\nStack: ");
  for (i = 1; i <= vm->cpu[vm->active].sp; i++) {
    if (i == vm->cpu[vm->active].sp)
      printf("[ TOS: %lld ]", (long long)vm->cpu[vm->active].data[i]);
    else
      printf("%lld ", (long long)vm->cpu[vm->active].data[i]);
  }
  printf("\n");
}

void dump_astack(NgaState *vm) {
  CELL i;
  if (vm->cpu[vm->active].rp == 0)  return;
  printf("\nAddress Stack: ");
  for (i = 1; i <= vm->cpu[vm->active].rp; i++) {
    if (i == vm->cpu[vm->active].rp)
      printf("[ TOS: %lld ]", (long long)vm->cpu[vm->active].address[i]);
    else
      printf("%lld ", (long long)vm->cpu[vm->active].address[i]);
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
  if (tests_enabled == 0) { return flag; }
  if (strcmp(buffer, vm->test_start) == 0) { flag = -1; }
  if (strcmp(buffer, vm->test_end) == 0)   { flag = -1; }
  return flag;
}


/*---------------------------------------------------------------------
  And now for the actual `include_file()` function.
  ---------------------------------------------------------------------*/

void read_line(NgaState *vm, FILE *file, char *token_buffer) {
  int ch = getc(file);
  int count = 0;
  while ((ch != 10) && (ch != 13) && (ch != EOF) && (ch != 0)) {
    token_buffer[count++] = ch;
    ch = getc(file);
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

void include_file(NgaState *vm, char *fname, int run_tests) {
  int inBlock = 0;                 /* Tracks status of in/out of block */
  int priorBlocks = 0;
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
  if (fp == NULL) {
    printf("File `%s` not found. Exiting.\n", fname);
    exit(1);
  }

  priorBlocks = vm->codeBlocks;
  vm->codeBlocks = 0;

  arp = vm->cpu[vm->active].rp;
  aip = vm->cpu[vm->active].ip;
  for(vm->cpu[vm->active].rp = 0; vm->cpu[vm->active].rp <= arp; vm->cpu[vm->active].rp++)
    ReturnStack[vm->cpu[vm->active].rp] = vm->cpu[vm->active].address[vm->cpu[vm->active].rp];
  vm->cpu[vm->active].rp = 0;

  vm->current_source++;
  strlcpy(vm->scripting_sources[vm->current_source], fname, 8192);

  vm->ignoreToEOF = 0;

  while (!feof(fp) && (vm->ignoreToEOF == 0)) { /* Loop through the file   */

    vm->ignoreToEOL = 0;

    offset = ftell(fp);
    read_line(vm, fp, line);
    at++;
    fseek(fp, offset, SEEK_SET);
    skip_indent(fp);

    tokens = count_tokens(line);

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
      read_line(vm, fp, line);
    }
  }

  vm->current_source--;
  vm->ignoreToEOF = 0;
  fclose(fp);
  if (vm->perform_abort == -1) {
    carry_out_abort(vm);
  }
  for(vm->cpu[vm->active].rp = 0; vm->cpu[vm->active].rp <= arp; vm->cpu[vm->active].rp++)
    vm->cpu[vm->active].address[vm->cpu[vm->active].rp] = ReturnStack[vm->cpu[vm->active].rp];
  vm->cpu[vm->active].rp = arp;
  vm->cpu[vm->active].ip = aip;

  if (vm->codeBlocks == 0) {
    printf("warning: no code blocks found!\n");
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

void initialize(NgaState *vm) {
  prepare_vm(vm);
  load_embedded_image(vm);
}


/*---------------------------------------------------------------------
  `arg_is()` exists to aid in readability. It compares the first actual
  command line argument to a string and returns a boolean flag.
  ---------------------------------------------------------------------*/

int arg_is(char *argv, char *t) {
  return strcmp(argv, t) == 0;
}


void help(char *exename) {
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
}

/* Signal Handler -----------------------------------------------------*/

#ifdef ENABLE_SIGNALS
static void sig_handler(int _)
{
  printf("\nCaught: %d\n", _);
  exit(1);
}
#endif

/* Main Entry Point ---------------------------------------------------*/
enum flags {
  FLAG_HELP, FLAG_INTERACTIVE,
};

int main(int argc, char **argv) {
  int i;
  int modes[16];
  NgaState *vm = calloc(sizeof(NgaState), sizeof(char));

#ifdef ENABLE_SIGNALS
  signal(SIGHUP, sig_handler);
  signal(SIGINT, sig_handler);
  signal(SIGILL, sig_handler);
  signal(SIGBUS, sig_handler);
  signal(SIGFPE, sig_handler);
#endif

  initialize(vm);               /* Initialize Nga & image    */

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

  strcpy(vm->code_start, "~~~");
  strcpy(vm->code_end,   "~~~");
  strcpy(vm->test_start, "```");
  strcpy(vm->test_end,   "```");

  /* Setup variables related to the scripting device */
  vm->currentLine = 0;           /* Current Line # for script */
  vm->current_source = 0;        /* Current file being run    */
  vm->perform_abort = 0;         /* Carry out abort procedure */
  vm->sys_argc = argc;           /* Point the global argc and */
  vm->sys_argv = argv;           /* argv to the actual ones   */
  strlcpy(vm->scripting_sources[0], "/dev/stdin", 8192);
  vm->ignoreToEOL = 0;
  vm->ignoreToEOF = 0;
  vm->codeBlocks = 0;

  if (argc >= 2 && argv[1][0] != '-') {
    update_rx(vm);
    include_file(vm, argv[1], 0);
    /* If no flags were passed,  */
    /* load the file specified,  */
    /* and exit                  */
    if (vm->cpu[vm->active].sp >= 1)  dump_stack(vm);
    exit(0);
  }

  /* Clear startup modes       */
  for (i = 0; i < 16; i++)
    modes[i] = 0;

  if (argc <= 1) modes[FLAG_INTERACTIVE] = 1;

  update_rx(vm);

  /* Process Arguments */
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      help(argv[0]);
      exit(0);
    } else if (strcmp(argv[i], "-i") == 0) {
      modes[FLAG_INTERACTIVE] = 1;
    } else if (strcmp(argv[i], "-f") == 0) {
      include_file(vm, argv[i + 1], 0);
      i++;
    } else if (strcmp(argv[i], "-u") == 0) {
      i++;
      load_image(vm, argv[i]);
      update_rx(vm);
    } else if (strcmp(argv[i], "-r") == 0) {
      i++;
      load_image(vm, argv[i]);
      modes[FLAG_INTERACTIVE] = 1;
      update_rx(vm);
    } else if (strcmp(argv[i], "-t") == 0) {
      include_file(vm, argv[i + 1], 1);
      i++;
    } else  if (arg_is(argv[i], "--code-start") || arg_is(argv[i], "-cs")) {
      i++;
      strcpy(vm->code_start, argv[i]);
    } else if (arg_is(argv[i], "--code-end") || arg_is(argv[i], "-ce")) {
      i++;
      strcpy(vm->code_end, argv[i]);
    } else if (arg_is(argv[i], "--test-start") || arg_is(argv[i], "-ts")) {
      i++;
      strcpy(vm->test_start, argv[i]);
    } else if (arg_is(argv[i], "--test-end") || arg_is(argv[i], "-te")) {
      i++;
      strcpy(vm->test_end, argv[i]);
    }
  }

  /* Run the Listener (if interactive mode was set) */
  if (modes[FLAG_INTERACTIVE] == 1) {
    execute(vm, 0);
  }

  /* Dump Stack */
  if (vm->cpu[vm->active].sp >= 1)  dump_stack(vm);

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
  vm->cpu[vm->active].sp--;
  return vm->cpu[vm->active].data[vm->cpu[vm->active].sp + 1];
}

void stack_push(NgaState *vm, CELL value) {
  vm->cpu[vm->active].sp++;
  vm->cpu[vm->active].data[vm->cpu[vm->active].sp] = value;
}


/*---------------------------------------------------------------------
  Strings are next. RETRO uses C-style NULL terminated strings. So I
  can easily inject or extract a string. Injection iterates over the
  string, copying it into the image. This also takes care to ensure
  that the NULL terminator is added.
  ---------------------------------------------------------------------*/

CELL string_inject(NgaState *vm, char *str, CELL buffer) {
  CELL m, i;
  if (!str) {
    vm->memory[buffer] = 0;
    return 0;
  }
  m = strlen(str);
  i = 0;
  while (m > 0) {
    vm->memory[buffer + i] = (CELL)str[i];
    vm->memory[buffer + i + 1] = 0;
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

char *string_extract(NgaState *vm, CELL at) {
  CELL starting = at;
  CELL i = 0;
  while(vm->memory[starting] && i < 8192)
    vm->string_data[i++] = (char)vm->memory[starting++];
  vm->string_data[i] = 0;
  return (char *)vm->string_data;
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

void update_rx(NgaState *vm) {
  vm->Dictionary = vm->memory[2];
  vm->interpret = vm->memory[5];
  vm->NotFound = vm->memory[6];
}

/*=====================================================================*/

void register_device(NgaState *vm, void *handler, void *query) {
  vm->IO_deviceHandlers[vm->devices] = handler;
  vm->IO_queryHandlers[vm->devices] = query;
  vm->devices++;
}

void load_embedded_image(NgaState *vm) {
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
    /* Read the file into memory */
    imageSize = fread(vm->memory, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
  return imageSize;
}

void prepare_vm(NgaState *vm) {
  vm->active = 0;
  vm->cpu[vm->active].ip = vm->cpu[vm->active].sp = vm->cpu[vm->active].rp = vm->cpu[vm->active].u = 0;
  vm->cpu[vm->active].active = -1;
  for (vm->cpu[vm->active].ip = 0; vm->cpu[vm->active].ip < IMAGE_SIZE; vm->cpu[vm->active].ip++)
    vm->memory[vm->cpu[vm->active].ip] = 0; /* NO - nop instruction */
  for (vm->cpu[vm->active].ip = 0; vm->cpu[vm->active].ip < STACK_DEPTH; vm->cpu[vm->active].ip++)
    vm->cpu[vm->active].data[vm->cpu[vm->active].ip] = 0;
  for (vm->cpu[vm->active].ip = 0; vm->cpu[vm->active].ip < ADDRESSES; vm->cpu[vm->active].ip++)
    vm->cpu[vm->active].address[vm->cpu[vm->active].ip] = 0;
}

void inst_no(NgaState *vm) {
}

void inst_li(NgaState *vm) {
  vm->cpu[vm->active].sp++;
  vm->cpu[vm->active].ip++;
  TOS = vm->memory[vm->cpu[vm->active].ip];
}

void inst_du(NgaState *vm) {
  vm->cpu[vm->active].sp++;
  vm->cpu[vm->active].data[vm->cpu[vm->active].sp] = NOS;
}

void inst_dr(NgaState *vm) {
  vm->cpu[vm->active].data[vm->cpu[vm->active].sp] = 0;
  vm->cpu[vm->active].sp--;
}

void inst_sw(NgaState *vm) {
  CELL a;
  a = TOS;
  TOS = NOS;
  NOS = a;
}

void inst_pu(NgaState *vm) {
  vm->cpu[vm->active].rp++;
  TORS = TOS;
  inst_dr(vm);
}

void inst_po(NgaState *vm) {
  vm->cpu[vm->active].sp++;
  TOS = TORS;
  vm->cpu[vm->active].rp--;
}

void inst_ju(NgaState *vm) {
  vm->cpu[vm->active].ip = TOS - 1;
  inst_dr(vm);
}

void inst_ca(NgaState *vm) {
  vm->cpu[vm->active].rp++;
  TORS = vm->cpu[vm->active].ip;
  vm->cpu[vm->active].ip = TOS - 1;
  inst_dr(vm);
}

void inst_cc(NgaState *vm) {
  CELL a, b;
  a = TOS; inst_dr(vm);  /* Target */
  b = TOS; inst_dr(vm);  /* Flag   */
  if (b != 0) {
    vm->cpu[vm->active].rp++;
    TORS = vm->cpu[vm->active].ip;
    vm->cpu[vm->active].ip = a - 1;
  }
}

void inst_re(NgaState *vm) {
  vm->cpu[vm->active].ip = TORS;
  vm->cpu[vm->active].rp--;
}

void inst_eq(NgaState *vm) {
  if (vm->cpu[vm->active].u != 0) {
    NOS = ((unsigned)NOS == (unsigned)TOS) ? -1 : 0;
    vm->cpu[vm->active].u = 0;
  } else {
    NOS = (NOS == TOS) ? -1 : 0;
  }
  inst_dr(vm);
}

void inst_ne(NgaState *vm) {
  if (vm->cpu[vm->active].u != 0) {
    NOS = ((unsigned)NOS != (unsigned)TOS) ? -1 : 0;
    vm->cpu[vm->active].u = 0;
  } else {
    NOS = (NOS != TOS) ? -1 : 0;
  }
  inst_dr(vm);
}

void inst_lt(NgaState *vm) {
  if (vm->cpu[vm->active].u != 0) {
    NOS = ((unsigned)NOS < (unsigned)TOS) ? -1 : 0;
    vm->cpu[vm->active].u = 0;
  } else {
    NOS = (NOS < TOS) ? -1 : 0;
  }
  inst_dr(vm);
}

void inst_gt(NgaState *vm) {
  if (vm->cpu[vm->active].u != 0) {
    NOS = ((unsigned)NOS > (unsigned)TOS) ? -1 : 0;
    vm->cpu[vm->active].u = 0;
  } else {
    NOS = (NOS > TOS) ? -1 : 0;
  }
  inst_dr(vm);
}

void inst_fe(NgaState *vm) {
  switch (TOS) {
    case -1: TOS = vm->cpu[vm->active].sp - 1; break;
    case -2: TOS = vm->cpu[vm->active].rp; break;
    case -3: TOS = IMAGE_SIZE; break;
    case -4: TOS = CELL_MIN; break;
    case -5: TOS = CELL_MAX; break;
    default: TOS = vm->memory[TOS]; break;
  }
}

void inst_st(NgaState *vm) {
  vm->memory[TOS] = NOS;
  inst_dr(vm);
  inst_dr(vm);
}

void inst_ad(NgaState *vm) {
  if (vm->cpu[vm->active].u != 0) {
    NOS = (unsigned)NOS + (unsigned)TOS;
    vm->cpu[vm->active].u = 0;
  } else {
    NOS += TOS;
  }
  inst_dr(vm);
}

void inst_su(NgaState *vm) {
  if (vm->cpu[vm->active].u != 0) {
    NOS = (unsigned)NOS - (unsigned)TOS;
    vm->cpu[vm->active].u = 0;
  } else {
    NOS -= TOS;
  }
  inst_dr(vm);
}

void inst_mu(NgaState *vm) {
  if (vm->cpu[vm->active].u != 0) {
    NOS = (unsigned)NOS * (unsigned)TOS;
    vm->cpu[vm->active].u = 0;
  } else {
    NOS *= TOS;
  }
  inst_dr(vm);
}

void inst_di(NgaState *vm) {
  CELL a, b;
  a = TOS;
  b = NOS;
  if (vm->cpu[vm->active].u != 0) {
    TOS = (unsigned)b / (unsigned)a;
    NOS = (unsigned)b % (unsigned)a;
    vm->cpu[vm->active].u = 0;
  } else {
    TOS = b / a;
    NOS = b % a;
  }
}

void inst_an(NgaState *vm) {
  NOS = TOS & NOS;
  inst_dr(vm);
}

void inst_or(NgaState *vm) {
  NOS = TOS | NOS;
  inst_dr(vm);
}

void inst_xo(NgaState *vm) {
  NOS = TOS ^ NOS;
  inst_dr(vm);
}

void inst_sh(NgaState *vm) {
  CELL y = TOS;
  CELL x = NOS;
  if (TOS < 0)
    NOS = NOS << (0 - TOS);
  else {
    if (vm->cpu[vm->active].u != 0) {
      NOS = (unsigned)x >> (unsigned)y;
      vm->cpu[vm->active].u = 0;
    } else {
      if (x < 0 && y > 0)
        NOS = x >> y | ~(~0U >> y);
      else
        NOS = x >> y;
    }
  }
  inst_dr(vm);
}

void inst_zr(NgaState *vm) {
  if (TOS == 0) {
    inst_dr(vm);
    vm->cpu[vm->active].ip = TORS;
    vm->cpu[vm->active].rp--;
  }
}

void inst_ha(NgaState *vm) {
  vm->cpu[vm->active].ip = IMAGE_SIZE;
  vm->cpu[vm->active].rp = 0;
}

void inst_ie(NgaState *vm) {
  stack_push(vm, vm->devices);
}

void inst_iq(NgaState *vm) {
  vm->IO_queryHandlers[stack_pop(vm)](vm);
}

void inst_ii(NgaState *vm) {
  vm->IO_deviceHandlers[stack_pop(vm)](vm);
}

Handler instructions[] = {
  inst_no, inst_li, inst_du, inst_dr, inst_sw, inst_pu, inst_po,
  inst_ju, inst_ca, inst_cc, inst_re, inst_eq, inst_ne, inst_lt,
  inst_gt, inst_fe, inst_st, inst_ad, inst_su, inst_mu, inst_di,
  inst_an, inst_or, inst_xo, inst_sh, inst_zr, inst_ha, inst_ie,
  inst_iq, inst_ii
};

void process_opcode(NgaState *vm, CELL opcode) {
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

int validate_opcode_bundle(CELL opcode) {
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

void process_opcode_bundle(NgaState *vm, CELL opcode) {
  CELL raw = opcode;
  int i;
  for (i = 0; i < 4; i++) {
    process_opcode(vm, raw & 0xFF);
    raw = raw >> 8;
  }
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
