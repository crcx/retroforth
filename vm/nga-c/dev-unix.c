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
  sp--;
}

void unix_system() {
  system(string_extract(stack_pop()));
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
  bsd_strlcpy(path, string_extract(stack_pop()), 1024);
  execl(path, path, (char *)0);
  stack_push(errno);
}

void unix_exec1() {
  char path[1025];
  char arg0[1025];
  bsd_strlcpy(arg0, string_extract(stack_pop()), 1024);
  bsd_strlcpy(path, string_extract(stack_pop()), 1024);
  execl(path, path, arg0, (char *)0);
  stack_push(errno);
}

void unix_exec2() {
  char path[1025];
  char arg0[1025], arg1[1025];
  bsd_strlcpy(arg1, string_extract(stack_pop()), 1024);
  bsd_strlcpy(arg0, string_extract(stack_pop()), 1024);
  bsd_strlcpy(path, string_extract(stack_pop()), 1024);
  execl(path, path, arg0, arg1, (char *)0);
  stack_push(errno);
}

void unix_exec3() {
  char path[1025];
  char arg0[1025], arg1[1025], arg2[1025];
  bsd_strlcpy(arg2, string_extract(stack_pop()), 1024);
  bsd_strlcpy(arg1, string_extract(stack_pop()), 1024);
  bsd_strlcpy(arg0, string_extract(stack_pop()), 1024);
  bsd_strlcpy(path, string_extract(stack_pop()), 1024);
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
  c = stack_pop();
  b = stack_pop();
  a = stack_pop();
  write(fileno(OpenFileHandles[c]), string_extract(a), b);
}

void unix_chdir() {
  chdir(string_extract(stack_pop()));
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


/*---------------------------------------------------------------------
  Faster verisons of `n:put` and `s:put`
  ---------------------------------------------------------------------*/

void unix_io_putn() {
  printf("%ld", (long)stack_pop());
}

void unix_io_puts() {
  printf("%s", string_extract(stack_pop()));
}

Handler UnixActions[] = {
  unix_system,    unix_fork,       unix_exec0,   unix_exec1,   unix_exec2,
  unix_exec3,     unix_exit,       unix_getpid,  unix_wait,    unix_kill,
  unix_open_pipe, unix_close_pipe, unix_write,   unix_chdir,   unix_getenv,
  unix_putenv,    unix_sleep,      unix_io_putn, unix_io_puts
};

void io_unix_query() {
  stack_push(1);
  stack_push(8);
}

void io_unix_handler() {
  UnixActions[stack_pop()]();
}
