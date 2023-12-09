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


#ifdef ENABLE_UNIX
#include <sys/wait.h>
#include <unistd.h>

/*--------------------------------------------------------------
  `unix_open_pipe()` is like `file_open()`, but for pipes. This
  pulls from the data stack:

  - mode       (number, TOS)
  - executable (string, NOS)

  Modes are:

  | Mode | Corresponds To | Description          |
  | ---- | -------------- | -------------------- |
  |  0   | r              | Open for reading     |
  |  1   | w              | Open for writing     |
  |  3   | r+             | Open for read/update |

  The file name should be a NULL terminated string. This will
  attempt to open the requested file and will return a handle
  (index number into the `OpenFileHandles` array).

  Once opened, you can use the standard file words to read/write
  to the process.
  ------------------------------------------------------------*/

V unix_open_pipe(NgaState *vm) {
  CELL slot, mode, name;
  char *request;
  const char *modes[] = {"r", "w", "", "r+"};
  slot = files_get_handle(vm);
  mode = stack_pop(vm);
  name = stack_pop(vm);
  request = string_extract(vm, name);
  if (slot > 0) {
    vm->OpenFileHandles[slot] = popen(request, modes[mode]);
  }
  if (vm->OpenFileHandles[slot] == NULL) {
    vm->OpenFileHandles[slot] = 0;
    slot = 0;
  }
  stack_push(vm, slot);
}

V unix_close_pipe(NgaState *vm) {
  pclose(vm->OpenFileHandles[TOS]);
  vm->OpenFileHandles[TOS] = 0;
  stack_pop(vm);
}

V unix_system(NgaState *vm) {
  int ignore = 0;
  ignore = system(string_extract(vm, stack_pop(vm)));
}

V unix_fork(NgaState *vm) {
  stack_push(vm, fork());
}

V unix_run_external(NgaState *vm) {
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


/*--------------------------------------------------------------
  UNIX provides `execl` to execute a file, with various forms
  for arguments provided.

  Retro wraps this in several functions, one for each number of
  passed arguments. See the Glossary for details on what each
  takes from the stack. Each of these will return the error code
  if the execution fails.
  ------------------------------------------------------------*/

V unix_exec0(NgaState *vm) {
  char path[1025];
  strlcpy(path, string_extract(vm, stack_pop(vm)), 1024);
  execl(path, path, (char *)0);
  stack_push(vm, errno);
}

V unix_exec1(NgaState *vm) {
  char path[1025];
  char arg0[1025];
  strlcpy(arg0, string_extract(vm, stack_pop(vm)), 1024);
  strlcpy(path, string_extract(vm, stack_pop(vm)), 1024);
  execl(path, path, arg0, (char *)0);
  stack_push(vm, errno);
}

V unix_exec2(NgaState *vm) {
  char path[1025];
  char arg0[1025], arg1[1025];
  strlcpy(arg1, string_extract(vm, stack_pop(vm)), 1024);
  strlcpy(arg0, string_extract(vm, stack_pop(vm)), 1024);
  strlcpy(path, string_extract(vm, stack_pop(vm)), 1024);
  execl(path, path, arg0, arg1, (char *)0);
  stack_push(vm, errno);
}

V unix_exec3(NgaState *vm) {
  char path[1025];
  char arg0[1025], arg1[1025], arg2[1025];
  strlcpy(arg2, string_extract(vm, stack_pop(vm)), 1024);
  strlcpy(arg1, string_extract(vm, stack_pop(vm)), 1024);
  strlcpy(arg0, string_extract(vm, stack_pop(vm)), 1024);
  strlcpy(path, string_extract(vm, stack_pop(vm)), 1024);
  execl(path, path, arg0, arg1, arg2, (char *)0);
  stack_push(vm, errno);
}

V unix_exit(NgaState *vm) { exit(stack_pop(vm)); }
V unix_getpid(NgaState *vm) { stack_push(vm, getpid()); }
V unix_wait(NgaState *vm) { int a; stack_push(vm, wait(&a)); }

V unix_kill(NgaState *vm) {
  CELL a = stack_pop(vm);
  kill(stack_pop(vm), a);
}

V unix_write(NgaState *vm) {
  ssize_t ignore;
  CELL c = stack_pop(vm);
  CELL b = stack_pop(vm);
  CELL a = stack_pop(vm);
  ignore = write(fileno(vm->OpenFileHandles[c]), string_extract(vm, a), b);
}

V unix_chdir(NgaState *vm) {
  int ignore = chdir(string_extract(vm, stack_pop(vm)));
}

V unix_getenv(NgaState *vm) {
  CELL a = stack_pop(vm);
  CELL b = stack_pop(vm);
  string_inject(vm, getenv(string_extract(vm, b)), a);
}

V unix_putenv(NgaState *vm) {
  putenv(string_extract(vm, stack_pop(vm)));
}

V unix_sleep(NgaState *vm) { sleep(stack_pop(vm)); }

Handler UnixActions[] = {
  unix_system,    unix_fork,   unix_exec0,     unix_exec1,
  unix_exec2,     unix_exec3,  unix_exit,      unix_getpid,
  unix_wait,      unix_kill,   unix_open_pipe, unix_close_pipe,
  unix_write,     unix_chdir,  unix_getenv,    unix_putenv,
  unix_sleep,     unix_run_external
};

V query_unix(NgaState *vm) {
  stack_push(vm, 3);
  stack_push(vm, DEVICE_UNIX);
}

V io_unix(NgaState *vm) {
  UnixActions[stack_pop(vm)](vm);
}
#endif
