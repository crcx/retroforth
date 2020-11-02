/* RETRO ------------------------------------------------------
  A personal, minimalistic forth
  Copyright (c) 2016 - 2020 Charles Childers

  This is a minimalist implementation of an interactive
  RETRO system with an embedded image and the listerner loop
  being written in RETRO.

  Building:

    cp ngaImage barebones.image
    retro-extend barebones.image interface/barebones.retro
    retro-embedimage barebones.image >vm/nga-c/barebones_image.c
    cd vm/nga-c
    make barebones
  ---------------------------------------------------------- */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#define CELL int32_t
#define CELL_MIN INT_MIN + 1
#define CELL_MAX INT_MAX - 1

#define IMAGE_SIZE   242000       /* Amount of RAM. 968kB by default.  */
#define ADDRESSES    256          /* Depth of address stack            */
#define STACK_DEPTH  128          /* Depth of data stack               */

CELL sp, rp, ip;                  /* Data, address, instruction pointers */
CELL data[STACK_DEPTH];           /* The data stack                    */
CELL address[ADDRESSES];          /* The address stack                 */
CELL memory[IMAGE_SIZE + 1];      /* The memory for the image          */

#define TOS  data[sp]             /* Shortcut for top item on stack    */
#define NOS  data[sp-1]           /* Shortcut for second item on stack */
#define TORS address[rp]          /* Shortcut for top item on address stack */

#define NUM_DEVICES  2

typedef void (*Handler)(void);

Handler IO_deviceHandlers[NUM_DEVICES + 1];
Handler IO_queryHandlers[NUM_DEVICES + 1];

#include "barebones_image.c"

CELL stack_pop();
void stack_push(CELL value);
void execute(CELL cell);
CELL load_image();
void prepare_vm();
void process_opcode(CELL opcode);
void process_opcode_bundle(CELL opcode);
int validate_opcode_bundle(CELL opcode);

CELL stack_pop() {
  sp--;
  return data[sp + 1];
}

void stack_push(CELL value) {
  sp++;
  data[sp] = value;
}

void generic_output() {
  putc(stack_pop(), stdout);
  fflush(stdout);
}

void generic_output_query() {
  stack_push(0);
  stack_push(0);
}

void generic_input() {
  stack_push(getc(stdin));
  if (TOS == 127) TOS = 8;
}

void generic_input_query() {
  stack_push(0);
  stack_push(1);
}

void execute(CELL cell) {
  CELL opcode;
  rp = 1;
  ip = cell;
  while (ip < IMAGE_SIZE) {
    opcode = memory[ip];
    if (validate_opcode_bundle(opcode) != 0) {
      process_opcode_bundle(opcode);
    } else {
      printf("Invalid instruction!\n");
      exit(1);
    }
    ip++;
    if (rp == 0)
      ip = IMAGE_SIZE;
  }
}

int main(int argc, char **argv) {
  IO_deviceHandlers[0] = generic_output;
  IO_deviceHandlers[1] = generic_input;
  IO_queryHandlers[0] = generic_output_query;
  IO_queryHandlers[1] = generic_input_query;
  prepare_vm();
  load_image();
  execute(0);
  exit(0);
}

#include "nga.c"
