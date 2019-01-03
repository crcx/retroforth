# Nga

## Overview

Nga is a minimalistic virtual machine emulating a dual stack computer
with a simple instruction set.

This is the specification and reference implementation of Nga. All code
provided is in the C language.

## Quick Overview of the VM Model

**Memory** is a single, linear addressing space of signed integer
values. Each location is called a **cell**. Addressing starts at zero
and counts up.

The VM does not expose any registers via the instruction set. This
implementation makes use of an *instruction pointer* or **IP**, *data
stack pointer* or **SP**, and *address stack pointer* or **RP**.

Nga provides two LIFO stacks. The primary one is for data and the
secondary one is used to hold return addresses for subroutine calls.
Stack depths may vary depending on the underlying hardware and
application needs.

Instructions each take one cell. The **LIT** instruction requires a
value in the following cell; it will push this value to the stack when
executed.

Instruction processing: the **IP** is incremented and the opcode at the
current address is invoked. This process then repeats. Execution ends
if the **END** instruction is run or end of memory is reached.

Endian: the image files are stored in little endian format.

I/O is not specified, but Nga does provide for adding this in a modular
manner, using three instructions to enumerate, query, and interact
with any devices provided.

## Legal

Nga derives from my earlier work on Ngaro. The following block lists
the people who helped work on the C implementation.

~~~
/* Nga ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Copyright (c) 2008 - 2019, Charles Childers
   Copyright (c) 2009 - 2010, Luke Parrish
   Copyright (c) 2010,        Marc Simpson
   Copyright (c) 2010,        Jay Skeer
   Copyright (c) 2011,        Kenneth Keating
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
~~~

## Boilerplate

Since the code is in C, we have to include some headers.

~~~
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
~~~

Nga uses a call table for dispatching instruction handlers. I define
a type for this here, before starting the actual code.

~~~
typedef void (*Handler)(void);
~~~

## Configuration

To make it easier to adapt Nga to a specific target, we keep some
important constants grouped together at the start of the file.

These defaults are targeted towards a 32-bit model, with several
megabytes of RAM.

For smaller targets, drop the IMAGE_SIZE considerably as that's the
biggest pool of memory needed. Alternately, for use on machines with
plenty of RAM, consider increasing these and reaping the benefits of
a much larger memory and stack space.

~~~
#define CELL         int32_t
#define IMAGE_SIZE   524288 * 16
#define ADDRESSES    2048
#define STACK_DEPTH  512
#define NUM_DEVICES  0
~~~

## Numbering The Instructions

For this implementation an enum is used to name each of the
instructions. For reference, here are the instructions and their
corresponding values (in decimal):

    0  nop      7  jump      14  gt        21  and      28  io query
    1  lit <v>  8  call      15  fetch     22  or       29  io interact
    2  dup      9  ccall     16  store     23  xor
    3  drop    10  return    17  add       24  shift
    4  swap    11  eq        18  sub       25  zret
    5  push    12  neq       19  mul       26  end
    6  pop     13  lt        20  divmod    27  io enumerate

~~~
enum vm_opcode {
  VM_NOP,  VM_LIT,    VM_DUP,   VM_DROP,    VM_SWAP,   VM_PUSH,  VM_POP,
  VM_JUMP, VM_CALL,   VM_CCALL, VM_RETURN,  VM_EQ,     VM_NEQ,   VM_LT,
  VM_GT,   VM_FETCH,  VM_STORE, VM_ADD,     VM_SUB,    VM_MUL,   VM_DIVMOD,
  VM_AND,  VM_OR,     VM_XOR,   VM_SHIFT,   VM_ZRET,   VM_END,   VM_IO_ENUM,
  VM_IO_QUERY,        VM_IO_INTERACT
};
#define NUM_OPS VM_IO_INTERACT + 1
~~~

## VM State

The VM state is held in a few global variables. (It'd be better to use
a struct here, as Ngaro does, but this makes everything else a bit less
readable.)

Some things to note:

The data stack (**data**), address stack (**address**), and memory
(**memory**) are simple linear arrays.

There are stack pointers (**sp** for **data** and **rp** for
**address**), and an instruction pointer (**ip**). These are *not*
exposed via the instruction set.

~~~
CELL sp, rp, ip;
CELL data[STACK_DEPTH];
CELL address[ADDRESSES];
CELL memory[IMAGE_SIZE + 1];
~~~

For I/O devices, create the dispatch tables.

~~~
Handler IO_deviceHandlers[NUM_DEVICES + 1];
Handler IO_queryHandlers[NUM_DEVICES + 1];
~~~

## A Little More Boilerplate

Here we have a few bits of shorthand that'll be handled by the C
preprocessor. These are used for readability purposes.

~~~
#define TOS  data[sp]
#define NOS  data[sp-1]
#define TORS address[rp]
~~~

## Loading an Image File

A standard image file is a raw memory dump of signed integer values.
(The size of these is determined by the value of CELL; Nga defaults
to 32-bit)

What we do here is:

* attempt to open the file
* use **fseek()** and **ftell()** to find the length of the file.
* divide the length by the size of a cell to determine the number of cells
* read the cells into memory (**image**)
* return the size of the data read (in bytes)

~~~
CELL ngaLoadImage(char *imageFile) {
  FILE *fp;
  CELL imageSize;
  long fileLen;

  if ((fp = fopen(imageFile, "rb")) != NULL) {
    /* Determine length (in cells) */
    fseek(fp, 0, SEEK_END);
    fileLen = ftell(fp) / sizeof(CELL);
    rewind(fp);

    /* Read the file into memory */
    imageSize = fread(&memory, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
  else {
    printf("Unable to find the ngaImage!\n");
    exit(1);
  }
  return imageSize;
}
~~~

## Preparations

This function initializes all of the variables and fills the arrays
with known values. Memory is filled with **VM_NOP** instructions; the
others are populated with zeros.

~~~
void ngaPrepare() {
  ip = sp = rp = 0;

  for (ip = 0; ip < IMAGE_SIZE; ip++)
    memory[ip] = VM_NOP;

  for (ip = 0; ip < STACK_DEPTH; ip++)
    data[ip] = 0;

  for (ip = 0; ip < ADDRESSES; ip++)
    address[ip] = 0;
}
~~~

## The Instructions

I've chosen to implement each instruction as a separate function. This
keeps them shorter, and lets me simplify the instruction processor
later on.

The **NOP** instruction does nothing.

~~~
void inst_nop() {
}
~~~

**LIT** is a special case: it's followed by a value to push to the
stack. This needs to increment the **sp** and **ip** and push the value
at the incremented **ip** to the stack.

~~~
void inst_lit() {
  sp++;
  ip++;
  TOS = memory[ip];
}
~~~

**DUP** duplicates the top item on the stack.

~~~
void inst_dup() {
  sp++;
  data[sp] = NOS;
}
~~~

**DROP** removes the top item from the stack.

~~~
void inst_drop() {
  data[sp] = 0;
   if (--sp < 0)
     ip = IMAGE_SIZE;
}
~~~

**SWAP** switches the top and second items on the stack.

~~~
void inst_swap() {
  CELL a;
  a = TOS;
  TOS = NOS;
  NOS = a;
}
~~~

**PUSH** moves the top value from the data stack to the address stack.

~~~
void inst_push() {
  rp++;
  TORS = TOS;
  inst_drop();
}
~~~

**POP** moves the top item on the address stack to the data stack.

~~~
void inst_pop() {
  sp++;
  TOS = TORS;
  rp--;
}
~~~

**JUMP** moves execution to the address on the top of the stack.

~~~
void inst_jump() {
  ip = TOS - 1;
  inst_drop();
}
~~~

**CALL** calls a subroutine at the address on the top of the stack.

~~~
void inst_call() {
  rp++;
  TORS = ip;
  ip = TOS - 1;
  inst_drop();
}
~~~

**CCALL** is a conditional call. It takes two values: a flag and a
pointer for an address to jump to if the flag is true.

A false flag is zero. Any other value is true.

Example:

    :t
      lit 100
      return
    :f
      lit 200
      return
    :main
      lit 1
      lit 2
      eq
      lit t
      ccall
      li f
      call
    end

~~~
void inst_ccall() {
  CELL a, b;
  a = TOS; inst_drop();  /* False */
  b = TOS; inst_drop();  /* Flag  */
  if (b != 0) {
    rp++;
    TORS = ip;
    ip = a - 1;
  }
}
~~~

**RETURN** ends a subroutine and returns flow to the instruction
following the last **CALL** or **CCALL**.

~~~
void inst_return() {
  ip = TORS;
  rp--;
}
~~~

**EQ** compares two values for equality and returns a flag.

~~~
void inst_eq() {
  NOS = (NOS == TOS) ? -1 : 0;
  inst_drop();
}
~~~

**NEQ** compares two values for inequality and returns a flag.

~~~
void inst_neq() {
  NOS = (NOS != TOS) ? -1 : 0;
  inst_drop();
}
~~~

**LT** compares two values for less than and returns a flag.

~~~
void inst_lt() {
  NOS = (NOS < TOS) ? -1 : 0;
  inst_drop();
}
~~~

**GT** compares two values for greater than and returns a flag.

~~~
void inst_gt() {
  NOS = (NOS > TOS) ? -1 : 0;
  inst_drop();
}
~~~

**FETCH** takes an address and returns the value stored there.

This doubles as a means of introspection into the VM state. Negative
addresses correspond to VM queries:

| Address | Returns             |
| ------- | ------------------- |
| -1      | Data stack depth    |
| -2      | Address stack depth |
| -3      | Maximum Image Size  |

An implementation may use negative values below -100 for implementation
specific inquiries.

~~~
void inst_fetch() {
  switch (TOS) {
    case -1: TOS = sp - 1; break;
    case -2: TOS = rp; break;
    case -3: TOS = IMAGE_SIZE; break;
    default: TOS = memory[TOS]; break;
  }
}
~~~

**STORE** stores a value into an address.

~~~
void inst_store() {
  memory[TOS] = NOS;
  inst_drop();
  inst_drop();
}
~~~

**ADD** adds two numbers together.

~~~
void inst_add() {
  NOS += TOS;
  inst_drop();
}
~~~

**SUB** subtracts two numbers.

~~~
void inst_sub() {
  NOS -= TOS;
  inst_drop();
}
~~~

**MUL** multiplies two numbers.

~~~
void inst_mul() {
  NOS *= TOS;
  inst_drop();
}
~~~

**DIVMOD** divides and returns the quotient and remainder.

~~~
void inst_divmod() {
  CELL a, b;
  a = TOS;
  b = NOS;
  TOS = b / a;
  NOS = b % a;
}
~~~

**AND** performs a bitwise AND operation.

    +----------------+
    | Before | After |
    | ------ | ----- |
    | -1     | -1    |
    | -1     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    |  0     |  0    |
    | -1     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    | -1     |  0    |
    |  0     |       |
    +----------------+

~~~
void inst_and() {
  NOS = TOS & NOS;
  inst_drop();
}
~~~

**OR** performs a bitwise OR operation.

    +----------------+
    | Before | After |
    | ------ | ----- |
    | -1     | -1    |
    | -1     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    | -1     | -1    |
    |  0     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    |  0     | -1    |
    | -1     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    |  0     |  0    |
    |  0     |       |
    +----------------+

~~~
void inst_or() {
  NOS = TOS | NOS;
  inst_drop();
}
~~~

**XOR** performs a bitwise XOR operation.

    +----------------+
    | Before | After |
    | ------ | ----- |
    | -1     |  0    |
    | -1     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    |  0     | -1    |
    | -1     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    | -1     | -1    |
    |  0     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    |  0     |  0    |
    |  0     |       |
    +----------------+

~~~
void inst_xor() {
  NOS = TOS ^ NOS;
  inst_drop();
}
~~~

**SHIFT** performs a bitwise arithmetic SHIFT operation.

This takes two values:

  xy

And returns a single one:

  z

If `y` is positive, this shifts right. If negative, it shifts
left.

~~~
void inst_shift() {
  CELL y = TOS;
  CELL x = NOS;
  if (TOS < 0)
    NOS = NOS << (TOS * -1);
  else {
    if (x < 0 && y > 0)
      NOS = x >> y | ~(~0U >> y);
    else
      NOS = x >> y;
  }
  inst_drop();
}
~~~

**ZRET** returns from a subroutine if the top item on the stack is
zero. If not, it acts like a **NOP** instead.

~~~
void inst_zret() {
  if (TOS == 0) {
    inst_drop();
    ip = TORS;
    rp--;
  }
}
~~~

**END** tells Nga that execution should end.

~~~
void inst_end() {
  ip = IMAGE_SIZE;
}
~~~

**I/O ENUMERATE** pushes a value to the stack indicating the number
of provided devices.

~~~
void inst_ie() {
  sp++;
  TOS = NUM_DEVICES;
}
~~~

**I/O QUERY** takes a value and returns two. The top stack item
is an identifer for the type of device attached. The second indicates
a device specific version number.

~~~
void inst_iq() {
  CELL Device = TOS;
  inst_drop();
  IO_queryHandlers[Device]();
}
~~~

**I/O INTERACT** takes a value indicating the desired device and
performs an action with the device.

~~~
void inst_ii() {
  CELL Device = TOS;
  inst_drop();
  IO_deviceHandlers[Device]();
}
~~~

## Instruction Handler

In the past I handled the instructions via a big switch. With Nga, I
changed this to use a jump table. This is significantly more concise
and makes maintenance easier overall.

So first up, the jump table itself. Just a list of pointers to the
instruction implementations, in the proper order.

~~~
Handler instructions[NUM_OPS] = {
  inst_nop, inst_lit, inst_dup, inst_drop, inst_swap, inst_push, inst_pop,
  inst_jump, inst_call, inst_ccall, inst_return, inst_eq, inst_neq, inst_lt,
  inst_gt, inst_fetch, inst_store, inst_add, inst_sub, inst_mul, inst_divmod,
  inst_and, inst_or, inst_xor, inst_shift, inst_zret, inst_end, inst_ie,
  inst_iq, inst_ii
};
~~~

And now **ngaProcessOpcode()** which calls the functions in the jump
table.

~~~
void ngaProcessOpcode(CELL opcode) {
  instructions[opcode]();
}
~~~

Nga also allows (optionally) support for packing multiple instructions
per cell. This has benefits on memory constained targets as four
instructions can be packed into each cell, reducing memory usage
significantly.

Consider:

    lit 100
    lit 200
    add

In a standard image this will be five cells. One for each instruction
and one for each data item. With packed instructions it would be three
cells: one for each four instructions (we only have three here, so the
unused one is a simple NOP) and one for each data item.

This implementation provides two functions for handling these. The
first takes a packed instruction and validates each instruction as
being valid. The second will process each of the stored opcodes.

Note for those using this: packing should stop when instructions that
modify the IP for flow control are used. These are:

* JUMP
* CCALL
* CALL
* RET
* ZRET

Nga will not stop processing code, but execution flow errors may arise
if the packing tool does not take these into account.

~~~
int ngaValidatePackedOpcodes(CELL opcode) {
  CELL raw = opcode;
  CELL current;
  int valid = -1;
  int i;
  for (i = 0; i < 4; i++) {
    current = raw & 0xFF;
    if (!(current >= 0 && current <= 29))
      valid = 0;
    raw = raw >> 8;
  }
  return valid;
}

void ngaProcessPackedOpcodes(CELL opcode) {
  CELL raw = opcode;
  int i;
  for (i = 0; i < 4; i++) {
    ngaProcessOpcode(raw & 0xFF);
    raw = raw >> 8;
  }
}
~~~

Nga is intended to be used as a part of a larger environment adding
host-specific I/O and functionality. For testing purposes, it is
possible to run non-interactive images by building defining STANDALONE
when compiling this source. A sample `main()` is included below showing
how to use this.

~~~
#ifdef STANDALONE
int main(int argc, char **argv) {
  ngaPrepare();
  if (argc == 2)
      ngaLoadImage(argv[1]);
  else
      ngaLoadImage("ngaImage");

  CELL opcode, i;

  ip = 0;
  while (ip < IMAGE_SIZE) {
    opcode = memory[ip];
    if (ngaValidatePackedOpcodes(opcode) != 0) {
      ngaProcessPackedOpcodes(opcode);
    } else if (opcode >= 0 && opcode < 27) {
      ngaProcessOpcode(opcode);
    } else {
      printf("Invalid instruction!\n");
      printf("At %d, opcode %d\n", ip, opcode);
      exit(1);
    }
    ip++;
  }

  for (i = 1; i <= sp; i++)
    printf("%d ", data[i]);
  printf("\n");
  exit(0);
}
#endif
