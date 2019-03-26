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

#ifndef IMAGEFUNCTIONS
#define IMAGEFUNCTIONS

#define TIB            1025
#define D_OFFSET_LINK     0
#define D_OFFSET_XT       1
#define D_OFFSET_CLASS    2
#define D_OFFSET_NAME     3

#define CELL         int32_t

#define IMAGE_SIZE   524288 * 8
#define STACK_DEPTH  4096
#define ADDRESSES    STACK_DEPTH * 3

#ifdef MEM1024K
#undef IMAGE_SIZE
#undef STACK_DEPTH
#undef ADDRESSES
#define IMAGE_SIZE   242000
#define STACK_DEPTH  128
#define ADDRESSES    256
#else
#endif

#ifdef MEM512K
#undef IMAGE_SIZE
#undef STACK_DEPTH
#undef ADDRESSES
#define IMAGE_SIZE   384000
#define STACK_DEPTH  128
#define ADDRESSES    256
#else
#endif

#ifdef MEM256K
#undef IMAGE_SIZE
#undef STACK_DEPTH
#undef ADDRESSES
#define IMAGE_SIZE   48000
#define STACK_DEPTH  128
#define ADDRESSES    256
#else
#endif

#ifdef MEM192K
#undef IMAGE_SIZE
#undef STACK_DEPTH
#undef ADDRESSES
#define IMAGE_SIZE   31500
#define STACK_DEPTH  128
#define ADDRESSES    256
#else
#endif

#ifdef MEM128K
#undef IMAGE_SIZE
#undef STACK_DEPTH
#undef ADDRESSES
#define IMAGE_SIZE   24000
#define STACK_DEPTH  128
#define ADDRESSES    256
#else
#endif


extern CELL sp, rp, ip;
extern CELL data[STACK_DEPTH];
extern CELL address[ADDRESSES];
extern CELL memory[IMAGE_SIZE + 1];

#define TOS  data[sp]
#define NOS  data[sp-1]
#define TORS address[rp]

extern CELL Dictionary;
extern CELL NotFound;
extern CELL Compiler;
extern CELL interpret;

CELL stack_pop();
void stack_push(CELL value);
CELL string_inject(char *str, CELL buffer);
char *string_extract(CELL at);
CELL d_link(CELL dt);
CELL d_xt(CELL dt);
CELL d_class(CELL dt);
CELL d_name(CELL dt);
CELL d_lookup(CELL Dictionary, char *name);
CELL d_xt_for(char *Name, CELL Dictionary);
void update_rx();
void execute(CELL cell);
void ngaProcessOpcode(CELL opcode);
void ngaProcessPackedOpcodes(CELL opcode);
int ngaValidatePackedOpcodes(CELL opcode);

#endif
