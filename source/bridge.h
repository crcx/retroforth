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

#include "nga.h"
#include "config.h"
#ifndef _BRIDGE_H
#define _BRIDGE_H


/* This assumes some knowledge of the ngaImage format for the
   Retro language. If things change there, these will need to
   be adjusted to match. */

#define TIB            1025
#define D_OFFSET_LINK     0
#define D_OFFSET_XT       1
#define D_OFFSET_CLASS    2
#define D_OFFSET_NAME     3

#ifdef ARGV
extern char **sys_argv;
extern int sys_argc;
#endif

extern CELL Dictionary, Heap, Compiler;
extern CELL notfound;

CELL stack_pop();
void stack_push(CELL value);
int string_inject(char *str, int buffer);
char *string_extract(int at);
int d_link(CELL dt);
int d_xt(CELL dt);
int d_class(CELL dt);
int d_name(CELL dt);
int d_lookup(CELL Dictionary, char *name);
CELL d_xt_for(char *Name, CELL Dictionary);
CELL d_class_for(char *Name, CELL Dictionary);
CELL ioGetFileHandle();
CELL ioOpenFile();
CELL ioReadFile();
CELL ioWriteFile();
CELL ioCloseFile();
CELL ioGetFilePosition();
CELL ioSetFilePosition();
CELL ioGetFileSize();
CELL ioDeleteFile();
void ioFlushFile();
void update_rx();
void execute(int cell);
void evaluate(char *s);
int not_eol(int ch);
void read_token(FILE *file, char *token_buffer, int echo);
char *read_token_str(char *s, char *token_buffer, int echo);

#endif
