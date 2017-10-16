/*  ____   ____ ______ ____    ___
    || \\ ||    | || | || \\  // \\
    ||_// ||==    ||   ||_// ((   ))
    || \\ ||___   ||   || \\  \\_//
    a personal, minimalistic forth

    This is an experimental curses-based interface to Retro. It is hoped
    that it will eventually be a suitable replacement for the older
    Listener.

    Copyright (c) 2016, 2017 Charles Childers
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <ncurses.h>

//LIBS m
//LIBS curses
//USES nga
//USES cursed-bridge
//HEAD config

WINDOW *output, *input, *stack, *back;

#include "nga.h"
#include "bridge.h"


void dump_stack() {
  CELL i;
  wclear(stack);
  if (sp == 0)
    return;
  for (i = sp; i > 0 && i != sp - 5; i--)
    if (i == sp)
      wprintw(stack, " [ %d ] ", data[i]);
    else
      wprintw(stack, " %d ", data[i]);
}

int main() {
  int ch;

  initscr();
  start_color();
  printw("first");
  refresh();
  cbreak();

  ngaPrepare();
  ngaLoadImage("ngaImage");
  update_rx();

  back = newwin(LINES - 1, COLS, 0, 0);
  output = newwin(LINES - 3, COLS - 2, 1, 1);
  input = newwin(1, COLS - 36, LINES - 1, 0);
  stack = newwin(1, COLS - 36, LINES - 1, COLS - 36);
  box(back, '|', '-');
  scrollok(output, TRUE);
  keypad(input, TRUE);

  wprintw(output, "RETRO 12 (rx-%d.%d)\n", memory[4] / 100, memory[4] % 100);
  wprintw(output, "%d MAX, TIB @ %d, Heap @ %d\n\n", IMAGE_SIZE, TIB, Heap);
  wprintw(output, "[ok] ");
  wrefresh(back);
  wrefresh(output);
  wrefresh(input);
  wrefresh(stack);
  doupdate();

  char c[1024];
  int n = 0;

  while ((ch = wgetch(input)) != 27) {
    if (ch == 10 || ch == 13 || ch == 32 || ch == 9) {
      wprintw(output, "%s ", c);
      evaluate(c);
      update_rx();
      if (memory[Compiler] == 0)
        wprintw(output, "\n[ok] ", c);
      dump_stack();
      wrefresh(output);
      wrefresh(stack);
      n = 0;
      wclear(input);
      wprintw(input, " ");
      wrefresh(input);
    } else {
      c[n++] = ch;
      c[n] = '\0';
    }
  }

  endwin();
  exit(0);
}

