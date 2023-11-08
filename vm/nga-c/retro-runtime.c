/*---------------------------------------------------------------------
  Copyright (c) 2008 - 2021, Charles Childers

  Portions are based on Ngaro, which was additionally copyrighted by
  the following:

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

#define TIB memory[7]             /* Location of TIB                   */

#define MAX_DEVICES      32
#define MAX_OPEN_FILES   32

/* Function Prototypes ----------------------------------------------- */
CELL stack_pop();
void stack_push(CELL value);
CELL string_inject(char *str, CELL buffer);
char *string_extract(CELL at);
void update_rx();
void include_file(char *fname, int run_tests);

void register_device(void *handler, void *query);

void io_output();           void query_output();
void io_keyboard();         void query_keyboard();
void query_filesystem();    void io_filesystem();
void io_clock();            void query_clock();
void io_scripting();        void query_scripting();
void io_rng();              void query_rng();

#ifdef ENABLE_UNIX
void query_unix();          void io_unix();
#endif

#ifdef ENABLE_FLOATS
void io_floatingpoint();    void query_floatingpoint();
#endif

#ifdef ENABLE_SOCKETS
void io_socket();           void query_socket();
#endif

void io_image();            void query_image();

void load_embedded_image(char *arg);
CELL load_image();
void prepare_vm();
void process_opcode_bundle(CELL opcode);
int validate_opcode_bundle(CELL opcode);

#ifdef NEEDS_STRL
size_t strlcat(char *dst, const char *src, size_t dsize);
size_t strlcpy(char *dst, const char *src, size_t dsize);
#endif

void prepare_vm();

void inst_no();  void inst_li();  void inst_du();
void inst_dr();  void inst_sw();  void inst_pu();
void inst_po();  void inst_ju();  void inst_ca();
void inst_cc();  void inst_re();  void inst_eq();
void inst_ne();  void inst_lt();  void inst_gt();
void inst_fe();  void inst_st();  void inst_ad();
void inst_su();  void inst_mu();  void inst_di();
void inst_an();  void inst_or();  void inst_xo();
void inst_sh();  void inst_zr();  void inst_ha();
void inst_ie();  void inst_iq();  void inst_ii();


/* Image, Stack, and VM variables ------------------------------------ */
CELL memory[IMAGE_SIZE + 1];      /* The memory for the image          */

#define TOS  cpu.data[cpu.sp]     /* Top item on stack                 */
#define NOS  cpu.data[cpu.sp-1]   /* Second item on stack              */
#define TORS cpu.address[cpu.rp]  /* Top item on address stack         */

struct NgaCore {
  CELL sp, rp, ip;                /* Stack & instruction pointers      */
  CELL data[STACK_DEPTH];         /* The data stack                    */
  CELL address[ADDRESSES];        /* The address stack                 */
} cpu;

int devices;                      /* The number of I/O devices         */


/* Markers for code & test blocks ------------------------------------ */
char code_start[33], code_end[33], test_start[33], test_end[33];

/* Populate The I/O Device Tables ------------------------------------ */
typedef void (*Handler)(void);
Handler IO_deviceHandlers[MAX_DEVICES];
Handler IO_queryHandlers[MAX_DEVICES];

/* Global Variables -------------------------------------------------- */
CELL Dictionary, NotFound, interpret;

char string_data[8192];
char **sys_argv;
int sys_argc;
char scripting_sources[64][8192];
int current_source;
int perform_abort;


/* Floating Point ---------------------------------------------------- */
#ifdef ENABLE_FLOATS
double Floats[256], AFloats[256];
CELL fsp, afsp;

void float_guard() {
  if (fsp < 0 || fsp > 255) {
    printf("\nERROR (nga/float_guard): Float Stack Limits Exceeded!\n");
    printf("At %lld, fsp = %lld\n", (long long)cpu.ip, (long long)fsp);
    exit(1);
  }
  if (afsp < 0 || afsp > 255) {
    printf("\nERROR (nga/float_guard): Alternate Float Stack Limits Exceeded!\n");
    printf("At %lld, afsp = %lld\n", (long long)cpu.ip, (long long)afsp);
    exit(1);
  }
}

/*---------------------------------------------------------------------
  The first two functions push a float to the stack and pop a value off
  the stack.
  ---------------------------------------------------------------------*/

void float_push(double value) {
  fsp++;
  float_guard();
  Floats[fsp] = value;
}

double float_pop() {
  fsp--;
  float_guard();
  return Floats[fsp + 1];
}

void float_to_alt() {
  afsp++;
  float_guard();
  AFloats[afsp] = float_pop();
}

void float_from_alt() {
  float_push(AFloats[afsp]);
  afsp--;
  float_guard();
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

void query_floatingpoint() {
  stack_push(1);
  stack_push(2);
}

void io_floatingpoint() {
  FloatHandlers[stack_pop()]();
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

void file_open() {
  CELL slot, mode, name;
  char *request;
  slot = files_get_handle();
  mode = stack_pop();
  name = stack_pop();
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
  CELL c;
  CELL slot = stack_pop();
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_read): Invalid file handle\n");
    exit(1);
  }
  c = fgetc(OpenFileHandles[slot]);
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
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_write): Invalid file handle\n");
    exit(1);
  }
  c = stack_pop();
  r = fputc(c, OpenFileHandles[slot]);
}


/*---------------------------------------------------------------------
  `file_close()` closes a file. This takes a file handle from the
  stack and does not return anything on the stack.
  ---------------------------------------------------------------------*/

void file_close() {
  CELL slot = stack_pop();
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

void file_get_position() {
  CELL slot = stack_pop();
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_get_position): Invalid file handle\n");
    exit(1);
  }
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

void file_get_size() {
  CELL slot, current, r, size;
  struct stat buffer;
  slot = stack_pop();
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
  stack_push((r == 0) ? size : 0);
}


/*---------------------------------------------------------------------
  `file_delete()` removes a file. This takes a file name (as a string)
  from the stack.
  ---------------------------------------------------------------------*/

void file_delete() {
  char *request;
  CELL name = stack_pop();
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

void query_filesystem() {
  stack_push(0);
  stack_push(4);
}

void io_filesystem() {
  FileActions[stack_pop()]();
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
  pclose(OpenFileHandles[TOS]);
  OpenFileHandles[TOS] = 0;
  stack_pop();
}

void unix_system() {
  int ignore = 0;
  ignore = system(string_extract(stack_pop()));
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
  strlcpy(path, string_extract(stack_pop()), 1024);
  execl(path, path, (char *)0);
  stack_push(errno);
}

void unix_exec1() {
  char path[1025];
  char arg0[1025];
  strlcpy(arg0, string_extract(stack_pop()), 1024);
  strlcpy(path, string_extract(stack_pop()), 1024);
  execl(path, path, arg0, (char *)0);
  stack_push(errno);
}

void unix_exec2() {
  char path[1025];
  char arg0[1025], arg1[1025];
  strlcpy(arg1, string_extract(stack_pop()), 1024);
  strlcpy(arg0, string_extract(stack_pop()), 1024);
  strlcpy(path, string_extract(stack_pop()), 1024);
  execl(path, path, arg0, arg1, (char *)0);
  stack_push(errno);
}

void unix_exec3() {
  char path[1025];
  char arg0[1025], arg1[1025], arg2[1025];
  strlcpy(arg2, string_extract(stack_pop()), 1024);
  strlcpy(arg1, string_extract(stack_pop()), 1024);
  strlcpy(arg0, string_extract(stack_pop()), 1024);
  strlcpy(path, string_extract(stack_pop()), 1024);
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
  ssize_t ignore;
  c = stack_pop();
  b = stack_pop();
  a = stack_pop();
  ignore = write(fileno(OpenFileHandles[c]), string_extract(a), b);
}

void unix_chdir() {
  int ignore;
  ignore = chdir(string_extract(stack_pop()));
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

Handler UnixActions[] = {
  unix_system,    unix_fork,       unix_exec0,   unix_exec1,   unix_exec2,
  unix_exec3,     unix_exit,       unix_getpid,  unix_wait,    unix_kill,
  unix_open_pipe, unix_close_pipe, unix_write,   unix_chdir,   unix_getenv,
  unix_putenv,    unix_sleep
};

void query_unix() {
  stack_push(2);
  stack_push(8);
}

void io_unix() {
  UnixActions[stack_pop()]();
}
#endif


/* Time and Date Functions --------------------------------------------*/
#ifdef ENABLE_CLOCK
void clock_time() {
  stack_push((CELL)time(NULL));
}

void clock_day() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_mday);
}

void clock_month() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_mon + 1);
}

void clock_year() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_year + 1900);
}

void clock_hour() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_hour);
}

void clock_minute() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_min);
}

void clock_second() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_sec);
}

void clock_day_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_mday);
}

void clock_month_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_mon + 1);
}

void clock_year_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_year + 1900);
}

void clock_hour_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_hour);
}

void clock_minute_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_min);
}

void clock_second_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_sec);
}

Handler ClockActions[] = {
  clock_time,
  clock_day,      clock_month,      clock_year,
  clock_hour,     clock_minute,     clock_second,
  clock_day_utc,  clock_month_utc,  clock_year_utc,
  clock_hour_utc, clock_minute_utc, clock_second_utc
};

void query_clock() {
  stack_push(0);
  stack_push(5);
}

void io_clock() {
  ClockActions[stack_pop()]();
}
#endif


/* Random Number Generator --------------------------------------------*/
#ifdef ENABLE_RNG
void io_rng() {
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
  stack_push((CELL)abs((CELL)r));
#else
  stack_push((CELL)llabs((CELL)r));
#endif
}

void query_rng() {
  stack_push(0);
  stack_push(10);
}
#endif


#ifdef ENABLE_SOCKETS
/*---------------------------------------------------------------------
  BSD Sockets
  ---------------------------------------------------------------------*/

int SocketID[16];
struct sockaddr_in Sockets[16];

struct addrinfo hints, *res;

void socket_getaddrinfo() {
  char host[1025], port[6];
  strlcpy(port, string_extract(stack_pop()), 5);
  strlcpy(host, string_extract(stack_pop()), 1024);
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
  int sock, port;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  sock = stack_pop();
  port = stack_pop();

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
#endif


/*---------------------------------------------------------------------
  Now on to I/O and extensions!
  ---------------------------------------------------------------------*/

void io_output() {
  putc(stack_pop(), stdout);
  fflush(stdout);
}

void query_output() {
  stack_push(0);
  stack_push(0);
}


/*=====================================================================*/

void io_keyboard() {
  stack_push(getc(stdin));
  if (TOS == 127) TOS = 8;
}

void query_keyboard() {
  stack_push(0);
  stack_push(1);
}

/*=====================================================================*/

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

void query_image() {
  stack_push(0);
  stack_push(1000);
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
#ifdef ENABLE_FLOATS
  fsp = 0;
  afsp = 0;
#endif

  if (current_source > 0) {
    scripting_abort();
    return;
  }

  perform_abort = 0;
  current_source = 0;
}

Handler ScriptingActions[] = {
  scripting_arg_count,     scripting_arg,
  scripting_include,       scripting_name,
  scripting_source,        scripting_line,
  scripting_ignore_to_eol, scripting_ignore_to_eof,
  scripting_abort
};

void query_scripting() {
  stack_push(2);
  stack_push(9);
}

void io_scripting() {
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

void execute(CELL cell) {
  CELL token;
  CELL opcode;
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

void evaluate(char *s) {
  if (strlen(s) == 0)  return;
  string_inject(s, TIB);
  stack_push(TIB);
  execute(interpret);
}


/*---------------------------------------------------------------------
  `read_token` reads a token from the specified file.  It will stop on
   a whitespace or newline. It also tries to handle backspaces, though
   the success of this depends on how your terminal is configured.
  ---------------------------------------------------------------------*/

int not_eol(int c) {
  return (c != 10) && (c != 13) && (c != 32) && (c != EOF) && (c != 0);
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
  int count = 1;
  while (*line++) {
    if (isspace(line[0]))
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
   strlcpy(scripting_sources[current_source], fname, 8192);

  ignoreToEOF = 0;

  while (!feof(fp) && (ignoreToEOF == 0)) { /* Loop through the file   */

    ignoreToEOL = 0;

    offset = ftell(fp);
    read_line(fp, line);
    fseek(fp, offset, SEEK_SET);

    tokens = count_tokens(line);

    while (tokens > 0 && ignoreToEOL == 0) {
      tokens--;
      read_token(fp, source);
      strlcpy(fence, source, 32); /* Copy the first three characters  */
      if (fence_boundary(fence, run_tests) == -1) {
        if (inBlock == 0)
          inBlock = 1;
        else
          inBlock = 0;
      } else {
        if (inBlock == 1) {
          currentLine = at;
          evaluate(source);
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
  `initialize()` sets up Nga and loads the image (from the array in
  `image.c`) to memory.
  ---------------------------------------------------------------------*/

void initialize() {
  prepare_vm();
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
  printf("Interactive Usage: %s [-h] [-i] [-f filename] [-t]\n\n", exename);
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
  printf("  -t\n");
  printf("    Run tests (in ``` blocks) in any loaded files\n\n");
}


/* Main Entry Point ---------------------------------------------------*/
enum flags {
  FLAG_HELP, FLAG_RUN_TESTS, FLAG_INCLUDE,
  FLAG_INTERACTIVE, FLAG_RUN,
};

int main(int argc, char **argv) {
  int i;
  int modes[16];
  char *files[16];
  int fsp;

  int run_tests;

  initialize();                           /* Initialize Nga & image    */
  load_embedded_image(argv[0]);

  register_device(io_output, query_output);
  register_device(io_keyboard, query_keyboard);
  register_device(io_filesystem, query_filesystem);
  register_device(io_image, query_image);
#ifdef ENABLE_FLOATS
  register_device(io_floatingpoint, query_floatingpoint);
#endif
#ifdef ENABLE_UNIX
  register_device(io_unix, query_unix);
#endif
#ifdef ENABLE_CLOCK
  register_device(io_clock, query_clock);
#endif
  register_device(io_scripting, query_scripting);
#ifdef ENABLE_RNG
  register_device(io_rng, query_rng);
#endif
#ifdef ENABLE_SOCKETS
  register_device(io_socket, query_socket);
#endif

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
  strlcpy(scripting_sources[0], "/dev/stdin", 8192);
  ignoreToEOL = 0;
  ignoreToEOF = 0;

  run_tests = 0;

  update_rx();
  execute(0);
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
  cpu.sp--;
  return cpu.data[cpu.sp + 1];
}

void stack_push(CELL value) {
  cpu.sp++;
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
  interpret = memory[5];
  NotFound = memory[6];
}

/*=====================================================================*/

void register_device(void *handler, void *query) {
  IO_deviceHandlers[devices] = handler;
  IO_queryHandlers[devices] = query;
  devices++;
}

CELL load_image(char *imageFile) {
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
    imageSize = fread(&memory, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
  return imageSize;
}

void prepare_vm() {
  cpu.ip = cpu.sp = cpu.rp = 0;
  for (cpu.ip = 0; cpu.ip < IMAGE_SIZE; cpu.ip++)
    memory[cpu.ip] = 0; /* NO - nop instruction */
  for (cpu.ip = 0; cpu.ip < STACK_DEPTH; cpu.ip++)
    cpu.data[cpu.ip] = 0;
  for (cpu.ip = 0; cpu.ip < ADDRESSES; cpu.ip++)
    cpu.address[cpu.ip] = 0;
}

void inst_no() {
}

void inst_li() {
  cpu.sp++;
  cpu.ip++;
  TOS = memory[cpu.ip];
}

void inst_du() {
  cpu.sp++;
  cpu.data[cpu.sp] = NOS;
}

void inst_dr() {
  cpu.data[cpu.sp] = 0;
  cpu.sp--;
}

void inst_sw() {
  CELL a;
  a = TOS;
  TOS = NOS;
  NOS = a;
}

void inst_pu() {
  cpu.rp++;
  TORS = TOS;
  inst_dr();
}

void inst_po() {
  cpu.sp++;
  TOS = TORS;
  cpu.rp--;
}

void inst_ju() {
  cpu.ip = TOS - 1;
  inst_dr();
}

void inst_ca() {
  cpu.rp++;
  TORS = cpu.ip;
  cpu.ip = TOS - 1;
  inst_dr();
}

void inst_cc() {
  CELL a, b;
  a = TOS; inst_dr();  /* Target */
  b = TOS; inst_dr();  /* Flag   */
  if (b != 0) {
    cpu.rp++;
    TORS = cpu.ip;
    cpu.ip = a - 1;
  }
}

void inst_re() {
  cpu.ip = TORS;
  cpu.rp--;
}

void inst_eq() {
  NOS = (NOS == TOS) ? -1 : 0;
  inst_dr();
}

void inst_ne() {
  NOS = (NOS != TOS) ? -1 : 0;
  inst_dr();
}

void inst_lt() {
  NOS = (NOS < TOS) ? -1 : 0;
  inst_dr();
}

void inst_gt() {
  NOS = (NOS > TOS) ? -1 : 0;
  inst_dr();
}

void inst_fe() {
  switch (TOS) {
    case -1: TOS = cpu.sp - 1; break;
    case -2: TOS = cpu.rp; break;
    case -3: TOS = IMAGE_SIZE; break;
    case -4: TOS = CELL_MIN; break;
    case -5: TOS = CELL_MAX; break;
    default: TOS = memory[TOS]; break;
  }
}

void inst_st() {
  memory[TOS] = NOS;
  inst_dr();
  inst_dr();
}

void inst_ad() {
  NOS += TOS;
  inst_dr();
}

void inst_su() {
  NOS -= TOS;
  inst_dr();
}

void inst_mu() {
  NOS *= TOS;
  inst_dr();
}

void inst_di() {
  CELL a, b;
  a = TOS;
  b = NOS;
  TOS = b / a;
  NOS = b % a;
}

void inst_an() {
  NOS = TOS & NOS;
  inst_dr();
}

void inst_or() {
  NOS = TOS | NOS;
  inst_dr();
}

void inst_xo() {
  NOS = TOS ^ NOS;
  inst_dr();
}

void inst_sh() {
  CELL y = TOS;
  CELL x = NOS;
  if (TOS < 0)
    NOS = NOS << (0 - TOS);
  else {
    if (x < 0 && y > 0)
      NOS = x >> y | ~(~0U >> y);
    else
      NOS = x >> y;
  }
  inst_dr();
}

void inst_zr() {
  if (TOS == 0) {
    inst_dr();
    cpu.ip = TORS;
    cpu.rp--;
  }
}

void inst_ha() {
  cpu.ip = IMAGE_SIZE;
}

void inst_ie() {
  stack_push(devices);
}

void inst_iq() {
  IO_queryHandlers[stack_pop()]();
}

void inst_ii() {
  IO_deviceHandlers[stack_pop()]();
}

Handler instructions[] = {
  inst_no, inst_li, inst_du, inst_dr, inst_sw, inst_pu, inst_po,
  inst_ju, inst_ca, inst_cc, inst_re, inst_eq, inst_ne, inst_lt,
  inst_gt, inst_fe, inst_st, inst_ad, inst_su, inst_mu, inst_di,
  inst_an, inst_or, inst_xo, inst_sh, inst_zr, inst_ha, inst_ie,
  inst_iq, inst_ii
};

void process_opcode(CELL opcode) {
#ifdef FAST
  switch (opcode) {
    case 0: break;
    case 1: inst_li(); break;
    case 2: inst_du(); break;
    case 3: inst_dr(); break;
    case 4: inst_sw(); break;
    case 5: inst_pu(); break;
    case 6: inst_po(); break;
    case 7: inst_ju(); break;
    case 8: inst_ca(); break;
    case 9: inst_cc(); break;
    case 10: inst_re(); break;
    case 11: inst_eq(); break;
    case 12: inst_ne(); break;
    case 13: inst_lt(); break;
    case 14: inst_gt(); break;
    case 15: inst_fe(); break;
    case 16: inst_st(); break;
    case 17: inst_ad(); break;
    case 18: inst_su(); break;
    case 19: inst_mu(); break;
    case 20: inst_di(); break;
    case 21: inst_an(); break;
    case 22: inst_or(); break;
    case 23: inst_xo(); break;
    case 24: inst_sh(); break;
    case 25: inst_zr(); break;
    case 26: inst_ha(); break;
    case 27: inst_ie(); break;
    case 28: inst_iq(); break;
    case 29: inst_ii(); break;
    default: break;
  }
#else
  if (opcode != 0)
    instructions[opcode]();
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

void process_opcode_bundle(CELL opcode) {
  CELL raw = opcode;
  int i;
  for (i = 0; i < 4; i++) {
    process_opcode(raw & 0xFF);
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
#pragma pack(push,1)
#pragma pack(pop)

#define EI_NIDENT       16

/* 32-bit ELF base types. */
typedef unsigned int Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Off;
typedef signed int Elf32_Sword;
typedef unsigned int Elf32_Word;

/* 64-bit ELF base types. */
typedef unsigned long long Elf64_Addr;
typedef unsigned short Elf64_Half;
typedef signed short Elf64_SHalf;
typedef unsigned long long Elf64_Off;
typedef signed int Elf64_Sword;
typedef unsigned int Elf64_Word;
typedef unsigned long long Elf64_Xword;
typedef signed long long Elf64_Sxword;

typedef struct elf32_hdr{
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half    e_type;
  Elf32_Half    e_machine;
  Elf32_Word    e_version;
  Elf32_Addr    e_entry;  /* Entry point */
  Elf32_Off e_phoff;
  Elf32_Off e_shoff;
  Elf32_Word    e_flags;
  Elf32_Half    e_ehsize;
  Elf32_Half    e_phentsize;
  Elf32_Half    e_phnum;
  Elf32_Half    e_shentsize;
  Elf32_Half    e_shnum;
  Elf32_Half    e_shstrndx;
} Elf32_Ehdr;

typedef struct elf32_shdr {
  Elf32_Word    sh_name;
  Elf32_Word    sh_type;
  Elf32_Word    sh_flags;
  Elf32_Addr    sh_addr;
  Elf32_Off sh_offset;
  Elf32_Word    sh_size;
  Elf32_Word    sh_link;
  Elf32_Word    sh_info;
  Elf32_Word    sh_addralign;
  Elf32_Word    sh_entsize;
} Elf32_Shdr;

typedef struct elf64_hdr {
  unsigned char e_ident[EI_NIDENT]; /* ELF "magic number" */
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;       /* Entry point virtual address */
  Elf64_Off e_phoff;        /* Program header table file offset */
  Elf64_Off e_shoff;        /* Section header table file offset */
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct elf64_shdr {
  Elf64_Word sh_name;       /* Section name, index in string tbl */
  Elf64_Word sh_type;       /* Type of section */
  Elf64_Xword sh_flags;     /* Miscellaneous section attributes */
  Elf64_Addr sh_addr;       /* Section virtual addr at execution */
  Elf64_Off sh_offset;      /* Section file offset */
  Elf64_Xword sh_size;      /* Size of section in bytes */
  Elf64_Word sh_link;       /* Index of another section */
  Elf64_Word sh_info;       /* Additional section information */
  Elf64_Xword sh_addralign; /* Section alignment */
  Elf64_Xword sh_entsize;   /* Entry size if section holds table */
} Elf64_Shdr;

void load_embedded_image(char *arg) {
  FILE* ElfFile = NULL;
  char* SectNames = NULL;
  Elf64_Ehdr elfHdr;
  Elf64_Shdr sectHdr;
  uint32_t idx;

  if((ElfFile = fopen(arg, "r")) == NULL) {
    perror("[E] Error opening file:");
    exit(1);
  }

  fread(&elfHdr, 1, sizeof(Elf64_Ehdr), ElfFile);
  fseek(ElfFile, elfHdr.e_shoff + elfHdr.e_shstrndx * sizeof(sectHdr), SEEK_SET);
  fread(&sectHdr, 1, sizeof(sectHdr), ElfFile);

  SectNames = malloc(sectHdr.sh_size);
  fseek(ElfFile, sectHdr.sh_offset, SEEK_SET);
  fread(SectNames, 1, sectHdr.sh_size, ElfFile);

  int a;

  for (idx = 0; idx < elfHdr.e_shnum; idx++)
  {
    const char* name = "";

    fseek(ElfFile, elfHdr.e_shoff + idx * sizeof(sectHdr), SEEK_SET);
    fread(&sectHdr, 1, sizeof(sectHdr), ElfFile);
    name = SectNames + sectHdr.sh_name;
    if (strcmp(name, ".ngaImage") == 0) {
      fseek(ElfFile, sectHdr.sh_offset, SEEK_SET);
      for (int i = 0; i < (int)sectHdr.sh_size; i++) {
        fread(&a, 1, sizeof(CELL), ElfFile);
        memory[i] = a;
      }
    }
  }
  return;
}
