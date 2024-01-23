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

#ifdef ENABLE_IOCTL
#include <sys/ioctl.h>
#include <termios.h>

void ioctl_get_terminal_size(NgaState *vm) {
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
  stack_push(vm, size.ws_row);
  stack_push(vm, size.ws_col);
}

void ioctl_set_character_breaking_mode(NgaState *vm) {
  struct termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &=(~ICANON & ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void ioctl_set_line_buffered_mode(NgaState *vm) {
  struct termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag |= ICANON;
  term.c_lflag |= ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

struct termios savedTermState;

void ioctl_save_current_state(NgaState *vm) {
  tcgetattr(STDIN_FILENO, &savedTermState);
}

void ioctl_restore_saved_state(NgaState *vm) {
  tcsetattr(STDIN_FILENO, TCSANOW, &savedTermState);
}

Handler IOCTLActions[] = {
  ioctl_get_terminal_size,
  ioctl_set_character_breaking_mode,
  ioctl_set_line_buffered_mode,
  ioctl_save_current_state,
  ioctl_restore_saved_state
};

V query_ioctl(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_IOCTL);
}

V io_ioctl(NgaState *vm) {
  IOCTLActions[stack_pop(vm)](vm);
}
#endif
