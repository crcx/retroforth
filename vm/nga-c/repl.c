/* RETRO is a clean, elegant, and pragmatic dialect of Forth.
   It provides a simple alternative for those willing to make a
   break from legacy systems.

   The language draws influences from many sources including
   traditional Forth systems, cmForth, colorForth, Factor, and
   Parable. It was designed to be easy to grasp and adapt to
   specific uses.

   The basic language is very portable and runs on a tiny
   virtual machine.

   This file contains a minimal implementation of the virtual
   machine.

   Copyright (c) 2008 - 2022, Charles Childers
   Copyright (c) 2009 - 2010, Luke Parrish
   Copyright (c) 2010,        Marc Simpson
   Copyright (c) 2010,        Jay Skeer
   Copyright (c) 2011,        Kenneth Keating
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

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
#define IMAGE_SIZE   65536       /* Amount of RAM, in cells */
#endif

#ifndef ADDRESSES
#define ADDRESSES    256          /* Depth of address stack */
#endif

#ifndef STACK_DEPTH
#define STACK_DEPTH  256          /* Depth of data stack */
#endif

CELL sp, rp, ip;                  /* Stack and instruction pointers */
CELL data[STACK_DEPTH];           /* The data stack          */
CELL address[ADDRESSES];          /* The address stack       */
CELL memory[IMAGE_SIZE + 1];      /* Image Memory            */

#define TOS  data[sp]             /* Top item on stack       */
#define NOS  data[sp-1]           /* Second item on stack    */
#define TORS address[rp]          /* Top item on address stack */

typedef void (*Handler)(void);

CELL stack_pop();
void stack_push(CELL value);
void execute(CELL cell);
void load_image(char *imageFile);
void prepare_vm();
void process_opcode_bundle(CELL opcode);

CELL stack_pop() {
  sp--;
  return data[sp + 1];
}

void stack_push(CELL value) {
  sp++;
  data[sp] = value;
}

void execute(CELL cell) {
  CELL opcode;
  rp = 1;
  ip = cell;
  while (ip < IMAGE_SIZE) {
    opcode = memory[ip];
    process_opcode_bundle(opcode);
    ip++;
    if (rp == 0)
      ip = IMAGE_SIZE;
  }
}

int main(int argc, char **argv) {
  prepare_vm();
  load_image("ngaImage");
  execute(0);
  exit(0);
}

void load_image(char *imageFile) {
  FILE *fp;
  long fileLen;
  if ((fp = fopen(imageFile, "rb")) != NULL) {
    fseek(fp, 0, SEEK_END);
    fileLen = ftell(fp) / sizeof(CELL);
    rewind(fp);
    fread(&memory, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
}

void prepare_vm() {
  ip = sp = rp = 0;
  for (ip = 0; ip < IMAGE_SIZE; ip++)
    memory[ip] = 0; /* NO - nop instruction */
  for (ip = 0; ip < STACK_DEPTH; ip++)
    data[ip] = 0;
  for (ip = 0; ip < ADDRESSES; ip++)
    address[ip] = 0;
}

void inst_no() {
}

void inst_li() {
  ip++;
  stack_push(memory[ip]);
}

void inst_du() {
  stack_push(TOS);
}

void inst_dr() {
  data[sp] = 0;
   if (--sp < 0)
     ip = IMAGE_SIZE;
}

void inst_sw() {
  CELL a;
  a = TOS;
  TOS = NOS;
  NOS = a;
}

void inst_pu() {
  rp++;
  TORS = TOS;
  inst_dr();
}

void inst_po() {
  stack_push(TORS);
  rp--;
}

void inst_ju() {
  ip = TOS - 1;
  inst_dr();
}

void inst_ca() {
  rp++;
  TORS = ip;
  ip = TOS - 1;
  inst_dr();
}

void inst_cc() {
  CELL a, b;
  a = TOS; inst_dr();  /* Target */
  b = TOS; inst_dr();  /* Flag   */
  if (b != 0) {
    rp++;
    TORS = ip;
    ip = a - 1;
  }
}

void inst_re() {
  ip = TORS;
  rp--;
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
    case -1: TOS = sp - 1; break;
    case -2: TOS = rp; break;
    case -3: TOS = IMAGE_SIZE; break;
    case -4: TOS = CELL_MIN; break;
    case -5: TOS = CELL_MAX; break;
    default: TOS = memory[TOS]; break;
  }
}

void inst_st() {
  if (TOS <= IMAGE_SIZE && TOS >= 0) {
    memory[TOS] = NOS;
    inst_dr();
    inst_dr();
  } else {
    ip = IMAGE_SIZE;
  }
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
    ip = TORS;
    rp--;
  }
}

void inst_ha() {
  ip = IMAGE_SIZE;
}

void inst_ie() {
  stack_push(2);
}

void inst_iq() {
  if (TOS == 0) {
    inst_dr();
    stack_push(0);
    stack_push(0);
  } else if (TOS == 1) {
    inst_dr();
    stack_push(1);
    stack_push(1);
  }
}

void inst_ii() {
  int c;
  if (TOS == 0) {
    inst_dr();
    putc(stack_pop(), stdout);
  } else if (TOS == 1) {
    c = getc(stdin);
    if (c < 0) exit(0);
    inst_dr();
    stack_push(c);
  } else {
    inst_dr();
  }
}

Handler instructions[] = {
  inst_no, inst_li, inst_du, inst_dr, inst_sw, inst_pu, inst_po,
  inst_ju, inst_ca, inst_cc, inst_re, inst_eq, inst_ne, inst_lt,
  inst_gt, inst_fe, inst_st, inst_ad, inst_su, inst_mu, inst_di,
  inst_an, inst_or, inst_xo, inst_sh, inst_zr, inst_ha, inst_ie,
  inst_iq, inst_ii
};

void process_opcode_bundle(CELL opcode) {
  instructions[opcode & 0xFF]();
  instructions[(opcode >> 8) & 0xFF]();
  instructions[(opcode >> 16) & 0xFF]();
  instructions[(opcode >> 24) & 0xFF]();
}
