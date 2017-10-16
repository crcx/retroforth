/*  ____   ____ ______ ____    ___
    || \\ ||    | || | || \\  // \\
    ||_// ||==    ||   ||_// ((   ))
    || \\ ||___   ||   || \\  \\_//
    a personal, minimalistic forth

    This implements a basic interface for interacting with the
    actual Retro language. It's intended to be used by the
    various interface layers and should work on most systems
    with a standard C compiler.

    Copyright (c) 2016, 2017 Charles Childers
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>

extern WINDOW *output, *input, *stack, *back;

#include "nga.h"
#include "bridge.h"

CELL Dictionary, Heap, Compiler;
CELL notfound;


/* Some I/O Parameters */

#define MAX_OPEN_FILES   128
#define NGURA_TTY_PUTC  1000
#define NGURA_TTY_GETC  1001
#define NGURA_FS_OPEN    118
#define NGURA_FS_CLOSE   119
#define NGURA_FS_READ    120
#define NGURA_FS_WRITE   121
#define NGURA_FS_TELL    122
#define NGURA_FS_SEEK    123
#define NGURA_FS_SIZE    124
#define NGURA_FS_DELETE  125
#define NGURA_FS_FLUSH   126


/* First, a couple of functions to simplify interacting with
   the stack. */

CELL stack_pop() {
  sp--;
  return data[sp + 1];
}

void stack_push(CELL value) {
  sp++;
  data[sp] = value;
}


/* Next, functions to translate C strings to/from Retro
   strings. */

int string_inject(char *str, int buffer) {
  int m = strlen(str);
  int i = 0;
  while (m > 0) {
    memory[buffer + i] = (CELL)str[i];
    memory[buffer + i + 1] = 0;
    m--; i++;
  }
  return buffer;
}

char string_data[8192];
char *string_extract(int at) {
  CELL starting = at;
  CELL i = 0;
  while(memory[starting] && i < 8192)
    string_data[i++] = (char)memory[starting++];
  string_data[i] = 0;
  return (char *)string_data;
}


/* Optional: FPU */
#ifdef FPU
#include "fpu.c"
#endif


/* Then accessor functions for dictionary fields. */

int d_link(CELL dt) {
  return dt + D_OFFSET_LINK;
}

int d_xt(CELL dt) {
  return dt + D_OFFSET_XT;
}

int d_class(CELL dt) {
  return dt + D_OFFSET_CLASS;
}

int d_name(CELL dt) {
  return dt + D_OFFSET_NAME;
}


/* With the dictionary accessors, some functions to actually
   lookup headers. */

int d_lookup(CELL Dictionary, char *name) {
  CELL dt = 0;
  CELL i = Dictionary;
  char *dname;
  while (memory[i] != 0 && i != 0) {
    dname = string_extract(d_name(i));
    if (strcmp(dname, name) == 0) {
      dt = i;
      i = 0;
    } else {
      i = memory[i];
    }
  }
  return dt;
}

CELL d_xt_for(char *Name, CELL Dictionary) {
  return memory[d_xt(d_lookup(Dictionary, Name))];
}

CELL d_class_for(char *Name, CELL Dictionary) {
  return memory[d_class(d_lookup(Dictionary, Name))];
}


/* Now for File I/O functions. These are adapted from
   Ngaro on Retro 11. */

FILE *nguraFileHandles[MAX_OPEN_FILES];

CELL nguraGetFileHandle() {
  CELL i;
  for(i = 1; i < MAX_OPEN_FILES; i++)
    if (nguraFileHandles[i] == 0)
      return i;
  return 0;
}

CELL nguraOpenFile() {
  CELL slot, mode, name;
  slot = nguraGetFileHandle();
  mode = data[sp]; sp--;
  name = data[sp]; sp--;
  char *request = string_extract(name);
  if (slot > 0) {
    if (mode == 0)  nguraFileHandles[slot] = fopen(request, "rb");
    if (mode == 1)  nguraFileHandles[slot] = fopen(request, "w");
    if (mode == 2)  nguraFileHandles[slot] = fopen(request, "a");
    if (mode == 3)  nguraFileHandles[slot] = fopen(request, "rb+");
  }
  if (nguraFileHandles[slot] == NULL) {
    nguraFileHandles[slot] = 0;
    slot = 0;
  }
  stack_push(slot);
  return slot;
}

CELL nguraReadFile() {
  CELL slot = stack_pop();
  CELL c = fgetc(nguraFileHandles[slot]);
  return feof(nguraFileHandles[slot]) ? 0 : c;
}

CELL nguraWriteFile() {
  CELL slot, c, r;
  slot = data[sp]; sp--;
  c = data[sp]; sp--;
  r = fputc(c, nguraFileHandles[slot]);
  return (r == EOF) ? 0 : 1;
}

CELL nguraCloseFile() {
  fclose(nguraFileHandles[data[sp]]);
  nguraFileHandles[data[sp]] = 0;
  sp--;
  return 0;
}

CELL nguraGetFilePosition() {
  CELL slot = data[sp]; sp--;
  return (CELL) ftell(nguraFileHandles[slot]);
}

CELL nguraSetFilePosition() {
  CELL slot, pos, r;
  slot = data[sp]; sp--;
  pos  = data[sp]; sp--;
  r = fseek(nguraFileHandles[slot], pos, SEEK_SET);
  return r;
}

CELL nguraGetFileSize() {
  CELL slot, current, r, size;
  slot = data[sp]; sp--;
  current = ftell(nguraFileHandles[slot]);
  r = fseek(nguraFileHandles[slot], 0, SEEK_END);
  size = ftell(nguraFileHandles[slot]);
  fseek(nguraFileHandles[slot], current, SEEK_SET);
  return (r == 0) ? size : 0;
}

CELL nguraDeleteFile() {
  CELL name = data[sp]; sp--;
  char *request = string_extract(name);
  return (unlink(request) == 0) ? -1 : 0;
}

void nguraFlushFile() {
  CELL slot;
  slot = data[sp]; sp--;
  fflush(nguraFileHandles[slot]);
}


/* Retro needs to track a few variables. This function is
   called as necessary to ensure that the interface stays
   in sync with the image state. */

void update_rx() {
  Dictionary = memory[2];
  Heap = memory[3];
  Compiler = d_xt_for("Compiler", Dictionary);
  notfound = d_xt_for("err:notfound", Dictionary);
}


/* The `execute` function runs a word in the Retro image.
   It also handles the additional I/O instructions. */

void execute(int cell) {
  CELL opcode;
  rp = 1;
  ip = cell;
  while (ip < IMAGE_SIZE) {
    if (ip == notfound) {
      wprintw(output, "%s ?\n", string_extract(TIB));
    }
    opcode = memory[ip];
    if (ngaValidatePackedOpcodes(opcode) != 0) {
      ngaProcessPackedOpcodes(opcode);
    } else if (opcode >= 0 && opcode < 27) {
      ngaProcessOpcode(opcode);
    } else {
      switch (opcode) {
        case NGURA_TTY_PUTC:  wprintw(output, "%c", stack_pop());        break;
        case NGURA_TTY_GETC:  stack_push(getc(stdin));                   break;
        case NGURA_FS_OPEN:   nguraOpenFile();                           break;
        case NGURA_FS_CLOSE:  nguraCloseFile();                          break;
        case NGURA_FS_READ:   stack_push(nguraReadFile());               break;
        case NGURA_FS_WRITE:  nguraWriteFile();                          break;
        case NGURA_FS_TELL:   stack_push(nguraGetFilePosition());        break;
        case NGURA_FS_SEEK:   nguraSetFilePosition();                    break;
        case NGURA_FS_SIZE:   stack_push(nguraGetFileSize());            break;
        case NGURA_FS_DELETE: nguraDeleteFile();                         break;
        case NGURA_FS_FLUSH:  nguraFlushFile();                          break;
#ifdef FPU
        case -6000: ngaFloatingPointUnit(); break;
#endif
        default:   wprintw(output, "Invalid instruction!\n");
                   wprintw(output, "At %d, opcode %d\n", ip, opcode);
                   exit(1);
      }
    }
    ip++;
    if (rp == 0)
      ip = IMAGE_SIZE;
  }
}


/* The `evaluate` function moves a token into the Retro
   token buffer, then calls the Retro `interpret` word
   to process it. */

void evaluate(char *s) {
  if (strlen(s) == 0)
    return;
  update_rx();
  CELL interpret = d_xt_for("interpret", Dictionary);
  string_inject(s, TIB);
  stack_push(TIB);
  execute(interpret);
}


/* `read_token` reads a token from the specified file.
   It will stop on a whitespace or newline. It also
   tries to handle backspaces, though the success of this
   depends on how your terminal is configured. */

int not_eol(int ch) {
  return (ch != (char)10) && (ch != (char)13) && (ch != (char)32) && (ch != EOF) && (ch != 0);
}

void read_token(FILE *file, char *token_buffer, int echo) {
  int ch = getc(file);
  if (echo != 0)
    putchar(ch);
  int count = 0;
  while (not_eol(ch))
  {
    if ((ch == 8 || ch == 127) && count > 0) {
      count--;
      if (echo != 0) {
        putchar(8);
        putchar(32);
        putchar(8);
      }
    } else {
      token_buffer[count++] = ch;
    }
    ch = getc(file);
    if (echo != 0)
      putchar(ch);
  }
  token_buffer[count] = '\0';
}


char *read_token_str(char *s, char *token_buffer, int echo) {
  int ch = (char)*s++;
  if (echo != 0)
    putchar(ch);
  int count = 0;
  while (not_eol(ch))
  {
    if ((ch == 8 || ch == 127) && count > 0) {
      count--;
      if (echo != 0) {
        putchar(8);
        putchar(32);
        putchar(8);
      }
    } else {
      token_buffer[count++] = ch;
    }
    ch = (char)*s++;
    if (echo != 0)
      putchar(ch);
  }
  token_buffer[count] = '\0';
  return s;
}

