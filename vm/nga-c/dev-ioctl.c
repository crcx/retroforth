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

#include "retro.h"

#ifdef ENABLE_IOCTL
#include <sys/ioctl.h>
#include <termios.h>

void ioctl_get_terminal_size(NgaState *vm) {
  struct winsize size;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == -1) {
    perror("ERROR (nga/ioctl): Unable to get terminal size");
    stack_push(vm, 0);
    stack_push(vm, 0);
    return;
  }
  stack_push(vm, size.ws_row);
  stack_push(vm, size.ws_col);
}

void ioctl_set_character_breaking_mode(NgaState *vm) {
  struct termios term;
  if (tcgetattr(STDIN_FILENO, &term) == -1) {
    perror("ERROR (nga/ioctl): Unable to get terminal settings");
    return;
  }
  term.c_lflag &=(~ICANON & ECHO);
  if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1) {
    perror("ERROR (nga/ioctl): Unable to set terminal settings");
  }
}

void ioctl_set_line_buffered_mode(NgaState *vm) {
  struct termios term;
  if (tcgetattr(STDIN_FILENO, &term) == -1) {
    perror("ERROR (nga/ioctl): Unable to get terminal settings");
    return;
  }
  term.c_lflag |= ICANON;
  term.c_lflag |= ECHO;
  if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1) {
    perror("ERROR (nga/ioctl): Unable to set terminal settings");
  }
}

struct termios savedTermState;
int savedTermStateValid;

void ioctl_save_current_state(NgaState *vm) {
  if (tcgetattr(STDIN_FILENO, &savedTermState) == -1) {
    savedTermStateValid = 0;
    perror("ERROR (nga/ioctl): Unable to save terminal settings");
    return;
  }
  savedTermStateValid = 1;
}

void ioctl_restore_saved_state(NgaState *vm) {
  if (!savedTermStateValid) {
    fprintf(stderr, "ERROR (nga/ioctl): No saved terminal settings\n");
    return;
  }
  if (tcsetattr(STDIN_FILENO, TCSANOW, &savedTermState) == -1) {
    perror("ERROR (nga/ioctl): Unable to restore terminal settings");
  }
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

V invalid_ioctl_action(NgaState *vm, CELL action) {
  printf("\nERROR (nga/ioctl): Invalid ioctl action %lld\n", (long long)action);
  ACTIVE.ip = IMAGE_SIZE;
  ACTIVE.rp = 0;
}

V io_ioctl(NgaState *vm) {
  CELL action = stack_pop(vm);
  CELL actions = sizeof(IOCTLActions) / sizeof(IOCTLActions[0]);
  if (action >= 0 && action < actions) {
    IOCTLActions[action](vm);
  } else {
    invalid_ioctl_action(vm, action);
  }
}
#endif
