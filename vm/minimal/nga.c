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

#define CELL int32_t
#define CELL_MIN INT_MIN + 1
#define CELL_MAX INT_MAX - 1

#define IMAGE_SIZE   65536       /* Amount of RAM, in cells */
#define ADDRESSES    256          /* Depth of address stack */
#define STACK_DEPTH  256          /* Depth of data stack */
#define IMAGE_CELLS  (IMAGE_SIZE + 1)

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

static void halt_with_error(void) {
  ip = IMAGE_SIZE;
}

static int valid_memory_address(CELL value) {
  return value >= 0 && value < IMAGE_CELLS;
}

static int can_push_data(void) { return sp < STACK_DEPTH - 1; }
static int can_pop_data(void) { return sp > 0; }
static int can_push_address(void) { return rp < ADDRESSES - 1; }
static int can_pop_address(void) { return rp > 0; }

CELL stack_pop() {
  if (!can_pop_data()) {
    halt_with_error();
    return 0;
  }
  sp--;
  return data[sp + 1];
}

void stack_push(CELL value) {
  if (!can_push_data()) {
    halt_with_error();
    return;
  }
  sp++;
  data[sp] = value;
}

void execute(CELL cell) {
  CELL opcode;
  rp = 1;
  ip = cell;
  while (ip >= 0 && ip < IMAGE_SIZE) {
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
    if (fileLen < 0 || fileLen > IMAGE_CELLS) {
      fclose(fp);
      return;
    }
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
  if (ip >= IMAGE_SIZE - 1 || !can_push_data()) { halt_with_error(); return; }
  ip++;
  stack_push(memory[ip]);
}

void inst_du() {
  if (!can_pop_data()) { halt_with_error(); return; }
  stack_push(TOS);
}

void inst_dr() {
  if (!can_pop_data()) { halt_with_error(); return; }
  data[sp] = 0;
   if (--sp < 0)
     ip = IMAGE_SIZE;
}

void inst_sw() {
  CELL a;
  if (sp < 2) { halt_with_error(); return; }
  a = TOS;
  TOS = NOS;
  NOS = a;
}

void inst_pu() {
  if (!can_push_address() || !can_pop_data()) { halt_with_error(); return; }
  rp++;
  TORS = TOS;
  inst_dr();
}

void inst_po() {
  if (!can_pop_address() || !can_push_data()) { halt_with_error(); return; }
  stack_push(TORS);
  rp--;
}

void inst_ju() {
  if (!can_pop_data() || TOS < 0 || TOS >= IMAGE_SIZE) { halt_with_error(); return; }
  ip = TOS - 1;
  inst_dr();
}

void inst_ca() {
  if (!can_push_address() || !can_pop_data() || TOS < 0 || TOS >= IMAGE_SIZE) { halt_with_error(); return; }
  rp++;
  TORS = ip;
  ip = TOS - 1;
  inst_dr();
}

void inst_cc() {
  CELL a, b;
  if (sp < 2) { halt_with_error(); return; }
  a = TOS; inst_dr();  /* Target */
  b = TOS; inst_dr();  /* Flag   */
  if (b != 0) {
    if (!can_push_address() || a < 0 || a >= IMAGE_SIZE) { halt_with_error(); return; }
    rp++;
    TORS = ip;
    ip = a - 1;
  }
}

void inst_re() {
  if (!can_pop_address() || TORS < 0 || TORS >= IMAGE_SIZE) { halt_with_error(); return; }
  ip = TORS;
  rp--;
}

void inst_eq() {
  if (sp < 2) { halt_with_error(); return; }
  NOS = (NOS == TOS) ? -1 : 0;
  inst_dr();
}

void inst_ne() {
  if (sp < 2) { halt_with_error(); return; }
  NOS = (NOS != TOS) ? -1 : 0;
  inst_dr();
}

void inst_lt() {
  if (sp < 2) { halt_with_error(); return; }
  NOS = (NOS < TOS) ? -1 : 0;
  inst_dr();
}

void inst_gt() {
  if (sp < 2) { halt_with_error(); return; }
  NOS = (NOS > TOS) ? -1 : 0;
  inst_dr();
}

void inst_fe() {
  if (!can_pop_data()) { halt_with_error(); return; }
  switch (TOS) {
    case -1: TOS = sp - 1; break;
    case -2: TOS = rp; break;
    case -3: TOS = IMAGE_SIZE; break;
    case -4: TOS = CELL_MIN; break;
    case -5: TOS = CELL_MAX; break;
    default: if (!valid_memory_address(TOS)) { halt_with_error(); return; }
             TOS = memory[TOS]; break;
  }
}

void inst_st() {
  if (sp < 2) { halt_with_error(); return; }
  if (valid_memory_address(TOS)) {
    memory[TOS] = NOS;
    inst_dr();
    inst_dr();
  } else {
    ip = IMAGE_SIZE;
  }
}

void inst_ad() {
  if (sp < 2) { halt_with_error(); return; }
  NOS = (CELL)((uint32_t)NOS + (uint32_t)TOS);
  inst_dr();
}

void inst_su() {
  if (sp < 2) { halt_with_error(); return; }
  NOS = (CELL)((uint32_t)NOS - (uint32_t)TOS);
  inst_dr();
}

void inst_mu() {
  if (sp < 2) { halt_with_error(); return; }
  NOS = (CELL)((uint32_t)NOS * (uint32_t)TOS);
  inst_dr();
}

void inst_di() {
  CELL a, b;
  if (sp < 2) { halt_with_error(); return; }
  a = TOS;
  b = NOS;
  if (a == 0 || (a == -1 && b == INT32_MIN)) { halt_with_error(); return; }
  TOS = b / a;
  NOS = b % a;
}

void inst_an() {
  if (sp < 2) { halt_with_error(); return; }
  NOS = TOS & NOS;
  inst_dr();
}

void inst_or() {
  if (sp < 2) { halt_with_error(); return; }
  NOS = TOS | NOS;
  inst_dr();
}

void inst_xo() {
  if (sp < 2) { halt_with_error(); return; }
  NOS = TOS ^ NOS;
  inst_dr();
}

void inst_sh() {
  CELL y = TOS;
  CELL x = NOS;
  if (sp < 2 || y < -31 || y > 31) { halt_with_error(); return; }
  if (TOS < 0)
    NOS = (CELL)((uint32_t)NOS << (0 - TOS));
  else {
    if (x < 0 && y > 0)
      NOS = x >> y | ~(~0U >> y);
    else
      NOS = x >> y;
  }
  inst_dr();
}

void inst_zr() {
  if (!can_pop_data()) { halt_with_error(); return; }
  if (TOS == 0) {
    if (!can_pop_address() || TORS < 0 || TORS >= IMAGE_SIZE) { halt_with_error(); return; }
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
  if (!can_pop_data()) { halt_with_error(); return; }
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
  if (!can_pop_data()) { halt_with_error(); return; }
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
  if ((opcode & 0xFF) >= 30 || ((opcode >> 8) & 0xFF) >= 30 ||
      ((opcode >> 16) & 0xFF) >= 30 || ((opcode >> 24) & 0xFF) >= 30) {
    halt_with_error();
    return;
  }
  instructions[opcode & 0xFF]();
  instructions[(opcode >> 8) & 0xFF]();
  instructions[(opcode >> 16) & 0xFF]();
  instructions[(opcode >> 24) & 0xFF]();
}
