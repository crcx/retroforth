/* RETRO ------------------------------------------------------
  A personal, minimalistic forth
  Copyright (c) 2016 - 2019 Charles Childers
  ---------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#include "strl.h"

typedef void (*Handler)(void);

/*---------------------------------------------------------------------
  First, a few things that relate to the Nga virtual machine that RETRO
  runs on.
  ---------------------------------------------------------------------*/

#define CELL         int32_t      /* Cell size (32 bit, signed integer */
#define IMAGE_SIZE   524288 * 48  /* Amount of RAM. 12MiB by default.  */
#define ADDRESSES    2048         /* Depth of address stack            */
#define STACK_DEPTH  512          /* Depth of data stack               */

extern CELL sp, rp, ip;             /* Stack & instruction pointers    */
extern CELL data[STACK_DEPTH];      /* The data stack                  */
extern CELL address[ADDRESSES];     /* The address stack               */
extern CELL memory[IMAGE_SIZE + 1]; /* The memory for the image        */

#define TOS  data[sp]             /* Shortcut for top item on stack    */
#define NOS  data[sp-1]           /* Shortcut for second item on stack */
#define TORS address[rp]          /* Shortcut for top item on address stack */

/*---------------------------------------------------------------------
  Function prototypes.
  ---------------------------------------------------------------------*/

CELL stack_pop();
void stack_push(CELL value);
char *string_extract(CELL at);
CELL string_inject(char *str, CELL buffer);

#define MAX_OPEN_FILES   128
extern FILE *ioFileHandles[MAX_OPEN_FILES];
CELL ioGetFileHandle();


/*---------------------------------------------------------------------
  `unixOpenPipe()` is like `ioOpenFile()`, but for pipes. This pulls
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
  into the `ioFileHandles` array).

  Once opened, you can use the standard file words to read/write to the
  process.
  ---------------------------------------------------------------------*/

void unixOpenPipe() {
  CELL slot, mode, name;
  char *request;
  slot = ioGetFileHandle();
  mode = stack_pop();
  name = stack_pop();
  request = string_extract(name);
  if (slot > 0) {
    if (mode == 0)  ioFileHandles[slot] = popen(request, "r");
    if (mode == 1)  ioFileHandles[slot] = popen(request, "w");
    if (mode == 3)  ioFileHandles[slot] = popen(request, "r+");
  }
  if (ioFileHandles[slot] == NULL) {
    ioFileHandles[slot] = 0;
    slot = 0;
  }
  stack_push(slot);
}


/*---------------------------------------------------------------------
  `unixClosePipe()` closes an open pipe. This takes a file handle from
  the stack.
  ---------------------------------------------------------------------*/

void unixClosePipe() {
  pclose(ioFileHandles[data[sp]]);
  ioFileHandles[data[sp]] = 0;
  sp--;
}


/*---------------------------------------------------------------------
  `unix_system()` executes a shell command. This takes a string and
  will execute it by calling the shell, returning to RRE after
  execution completes.
  ---------------------------------------------------------------------*/

void unix_system() {
  system(string_extract(stack_pop()));
}


/*---------------------------------------------------------------------
  `unix_fork()` creates a new process. This returns a new process ID on
  the stack.
  ---------------------------------------------------------------------*/

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


/*---------------------------------------------------------------------
  `unix_exit()` exits RRE with a return code of the top value on the
  stack.
  ---------------------------------------------------------------------*/

void unix_exit() {
  exit(stack_pop());
}


/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/

void unix_getpid() {
  stack_push(getpid());
}


/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/

void unix_wait() {
  CELL a;
  stack_push(wait(&a));
}


/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/

void unix_kill() {
  CELL a;
  a = stack_pop();
  kill(stack_pop(), a);
}


/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/

void unix_write() {
  CELL a, b, c;
  c = stack_pop();
  b = stack_pop();
  a = stack_pop();
  write(fileno(ioFileHandles[c]), string_extract(a), b);
}


/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/

void unix_chdir() {
  chdir(string_extract(stack_pop()));
}


/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/

void unix_getenv() {
  CELL a, b;
  a = stack_pop();
  b = stack_pop();
  string_inject(getenv(string_extract(b)), a);
}


/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/

void unix_putenv() {
  putenv(string_extract(stack_pop()));
}


/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/

void unix_sleep() {
  sleep(stack_pop());
}


/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/

void unix_io_putn() {
  printf("%ld", (long)stack_pop());
}

/*---------------------------------------------------------------------
  ---------------------------------------------------------------------*/

void unix_io_puts() {
  printf("%s", string_extract(stack_pop()));
}


/*---------------------------------------------------------------------
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
  stack_push((CELL)localtime(&t)->tm_mon);
}

void unix_time_year() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_year);
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


Handler UnixActions[] = {
  unix_system, unix_fork, unix_exec0, unix_exec1, unix_exec2,
  unix_exec3, unix_exit, unix_getpid, unix_wait, unix_kill,
  unixOpenPipe, unixClosePipe, unix_write, unix_chdir,
  unix_getenv, unix_putenv, unix_sleep, unix_io_putn, unix_io_puts,
  unix_time, unix_time_day, unix_time_month, unix_time_year,
  unix_time_hour, unix_time_minute, unix_time_second
};

void io_unix_query() {
  stack_push(0);
  stack_push(8);
}

void io_unix_handler() {
  UnixActions[stack_pop()]();
}
