/* RETRO/Native (x86) -----------------------------------------
  A personal, minimalistic forth
  Copyright (c) 2016 - 2019 Charles Childers
  -----------------------------------------------------------*/

#include <sys/types.h>
#include <limits.h>
#include "image.c"

/*-------------------------------------------------------------
  Next we get into some things that relate to the Nga virtual
  machine that RETRO runs on.
  -----------------------------------------------------------*/

#ifndef BIT64
#define CELL int32_t
#define CELL_MIN INT_MIN + 1
#define CELL_MAX INT_MAX - 1
#else
#define CELL int64_t
#define CELL_MIN LLONG_MIN + 1
#define CELL_MAX LLONG_MAX - 1
#endif

#define IMAGE_SIZE   524288 * 8  /* Amount of RAM.           */
#define ADDRESSES    1024        /* Depth of address stack   */
#define STACK_DEPTH  128         /* Depth of data stack      */

CELL sp, rp, ip;                 /* Stack & Instruction Ptrs */
CELL data[STACK_DEPTH];          /* The data stack           */
CELL address[ADDRESSES];         /* The address stack        */
CELL memory[IMAGE_SIZE + 1];     /* The memory for the image */

#define TOS  data[sp]
#define NOS  data[sp-1]
#define TORS address[rp]

#define NUM_DEVICES  3

typedef void (*Handler)(void);

Handler IO_deviceHandlers[NUM_DEVICES + 1];
Handler IO_queryHandlers[NUM_DEVICES + 1];

/*-------------------------------------------------------------
  Function prototypes.
  -----------------------------------------------------------*/

void execute(int cell);
CELL ngaLoadImage(char *imageFile);
void ngaPrepare();
void ngaProcessOpcode(CELL opcode);
void ngaProcessPackedOpcodes(CELL opcode);
int ngaValidatePackedOpcodes(CELL opcode);
CELL stack_pop();
void stack_push(CELL value);

int getchar(void);

int remap(int c) {
  int a = c;
#ifdef USE_DVORAK
  switch (c) {
    case 'q': a = '\''; break;   case 'Q': a = '"'; break;
    case 'w': a = ','; break;    case 'W': a = '<'; break;
    case 'e': a = '.'; break;    case 'E': a = '>'; break;
    case 'r': a = 'p'; break;    case 'R': a = 'P'; break;
    case 't': a = 'y'; break;    case 'T': a = 'Y'; break;
    case 'y': a = 'f'; break;    case 'Y': a = 'F'; break;
    case 'u': a = 'g'; break;    case 'U': a = 'G'; break;
    case 'i': a = 'c'; break;    case 'I': a = 'C'; break;
    case 'o': a = 'r'; break;    case 'O': a = 'R'; break;
    case 'p': a = 'l'; break;    case 'P': a = 'L'; break;
    case '[': a = '/'; break;    case '{': a = '?'; break;
    case ']': a = '='; break;    case '}': a = '+'; break;
    case 'a': a = 'a'; break;    case 'A': a = 'A'; break;
    case 's': a = 'o'; break;    case 'S': a = 'O'; break;
    case 'd': a = 'e'; break;    case 'D': a = 'E'; break;
    case 'f': a = 'u'; break;    case 'F': a = 'U'; break;
    case 'g': a = 'i'; break;    case 'G': a = 'I'; break;
    case 'h': a = 'd'; break;    case 'H': a = 'D'; break;
    case 'j': a = 'h'; break;    case 'J': a = 'H'; break;
    case 'k': a = 't'; break;    case 'K': a = 'T'; break;
    case 'l': a = 'n'; break;    case 'L': a = 'N'; break;
    case ';': a = 's'; break;    case ':': a = 'S'; break;
    case '\'': a = '-'; break;   case '"': a = '_'; break;
    case 'z': a = ';'; break;    case 'Z': a = ':'; break;
    case 'x': a = 'q'; break;    case 'X': a = 'Q'; break;
    case 'c': a = 'j'; break;    case 'C': a = 'J'; break;
    case 'v': a = 'k'; break;    case 'V': a = 'K'; break;
    case 'b': a = 'x'; break;    case 'B': a = 'X'; break;
    case 'n': a = 'b'; break;    case 'N': a = 'B'; break;
    case 'm': a = 'm'; break;    case 'M': a = 'M'; break;
    case ',': a = 'w'; break;    case '<': a = 'W'; break;
    case '.': a = 'v'; break;    case '>': a = 'V'; break;
    case '/': a = 'z'; break;    case '?': a = 'Z'; break;
    case '-': a = '['; break;    case '_': a = '{'; break;
    case '=': a = ']'; break;    case '+': a = '}'; break;
  }
#endif
  return a;
}

unsigned char inportb(unsigned int port)
{
   unsigned char ret;
   asm volatile ("inb %%dx,%%al":"=a" (ret):"d" (port));
   return ret;
}

void outportb(unsigned int port,unsigned char value)
{
   asm volatile ("outb %%al,%%dx": :"d" (port), "a" (value));
}

void store() {
  long address = stack_pop();
  long value   = stack_pop();
  *((int*)address) = value;
}

void fetch() {
  stack_push(*((int*)stack_pop()));
}

void storeb() {
  long address = stack_pop();
  long value   = stack_pop();
  *((char *)address) = (char)value;
}

void fetchb() {
  stack_push((long)*((char*)stack_pop()) & 0xFF);
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm volatile ( "inw %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline void outw(uint16_t port, uint16_t val)
{
    asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}




/*-------------------------------------------------------------
  Stack push/pop is easy. I could avoid these, but it aids in
  keeping the code readable, so it's worth the slight overhead.
  -----------------------------------------------------------*/

CELL stack_pop() {
  sp--;
  return data[sp + 1];
}

void stack_push(CELL value) {
  sp++;
  data[sp] = value;
}


/*-------------------------------------------------------------
  This is an implementation of the generic output device. It's
  set to ignore data passed into it. (The RETRO/Native image
  will use its own driver instead.)
  -----------------------------------------------------------*/

void generic_output() {
  stack_pop();
}

void generic_output_query() {
  stack_push(0);
  stack_push(0);
}


void portio() {
  CELL p, v;
  switch (stack_pop()) {
    case 0: p = stack_pop();
            stack_push((CELL)inportb((unsigned int)p));
            break;
    case 1: p = stack_pop();
            v = stack_pop();
            outportb((unsigned int)p, (unsigned char)v);
            break;
    case 2: store();
            break;
    case 3: fetch();
            break;
    case 4: storeb();
            break;
    case 5: fetchb();
            break;
    case 6: stack_push((CELL)inw((uint16_t)stack_pop()));
            break;
    case 7: p = stack_pop();
            v = stack_pop();
            outw((uint16_t)p, (uint16_t)v);
            break;
  }
}

void portio_query() {
  stack_push(0);
  stack_push(2000);
}

void generic_input() {
  stack_push(remap(getchar()));
  if (TOS == 127) TOS = 8;
}

void generic_input_query() {
  stack_push(0);
  stack_push(1);
}

/*-------------------------------------------------------------
  With these out of the way, I implement `execute`, which takes
  an address and runs the code at it.

  This will also exit if the address stack depth is zero
  (meaning that the word being run, and its dependencies) are
  finished.
  -----------------------------------------------------------*/

void execute(int cell) {
  CELL opcode;
  rp = 1;
  ip = cell;
  while (ip < IMAGE_SIZE) {
    opcode = memory[ip];
    if (ngaValidatePackedOpcodes(opcode) != 0) {
      ngaProcessPackedOpcodes(opcode);
    } else {
      while(1);
    }
    ip++;
    if (rp == 0)
      ip = IMAGE_SIZE;
  }
}


/*-------------------------------------------------------------
  The `main()` routine. This sets up the Nga VM, loads the
  image, and then transfers control to it.
  -----------------------------------------------------------*/

int main(int argc, char **argv) {
  IO_deviceHandlers[0] = generic_output;
  IO_queryHandlers[0] = generic_output_query;
  IO_deviceHandlers[1] = generic_input;
  IO_queryHandlers[1] = generic_input_query;
  IO_deviceHandlers[2] = portio;
  IO_queryHandlers[2] = portio_query;
  while (1) {
    ngaPrepare();
    for (CELL i = 0; i < ngaImageCells; i++)
      memory[i] = ngaImage[i];
    execute(0);
  }
}


/* Nga -------------------------------------------------------
   Copyright (c) 2008 - 2019, Charles Childers
   Copyright (c) 2009 - 2010, Luke Parrish
   Copyright (c) 2010,        Marc Simpson
   Copyright (c) 2010,        Jay Skeer
   Copyright (c) 2011,        Kenneth Keating
   ----------------------------------------------------------*/

enum vm_opcode {
  VM_NOP,  VM_LIT,    VM_DUP,   VM_DROP,    VM_SWAP,   VM_PUSH,  VM_POP,
  VM_JUMP, VM_CALL,   VM_CCALL, VM_RETURN,  VM_EQ,     VM_NEQ,   VM_LT,
  VM_GT,   VM_FETCH,  VM_STORE, VM_ADD,     VM_SUB,    VM_MUL,   VM_DIVMOD,
  VM_AND,  VM_OR,     VM_XOR,   VM_SHIFT,   VM_ZRET,   VM_HALT,  VM_IE,
  VM_IQ,   VM_II
};
#define NUM_OPS VM_II + 1

#ifndef NUM_DEVICES
#define NUM_DEVICES 0
#endif

CELL ngaLoadImage(char *imageFile) {
  CELL i;
  for (i = 0; i < ngaImageCells; i++)
    memory[i] = ngaImage[i];
  return i;
}

void ngaPrepare() {
  ip = sp = rp = 0;
  for (ip = 0; ip < IMAGE_SIZE; ip++)
    memory[ip] = VM_NOP;
  for (ip = 0; ip < STACK_DEPTH; ip++)
    data[ip] = 0;
  for (ip = 0; ip < ADDRESSES; ip++)
    address[ip] = 0;
}

void inst_nop() {
}

void inst_lit() {
  sp++;
  ip++;
  TOS = memory[ip];
}

void inst_dup() {
  sp++;
  data[sp] = NOS;
}

void inst_drop() {
  data[sp] = 0;
   if (--sp < 0)
     ip = IMAGE_SIZE;
}

void inst_swap() {
  CELL a;
  a = TOS;
  TOS = NOS;
  NOS = a;
}

void inst_push() {
  rp++;
  TORS = TOS;
  inst_drop();
}

void inst_pop() {
  sp++;
  TOS = TORS;
  rp--;
}

void inst_jump() {
  ip = TOS - 1;
  inst_drop();
}

void inst_call() {
  rp++;
  TORS = ip;
  ip = TOS - 1;
  inst_drop();
}

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

void inst_return() {
  ip = TORS;
  rp--;
}

void inst_eq() {
  NOS = (NOS == TOS) ? -1 : 0;
  inst_drop();
}

void inst_neq() {
  NOS = (NOS != TOS) ? -1 : 0;
  inst_drop();
}

void inst_lt() {
  NOS = (NOS < TOS) ? -1 : 0;
  inst_drop();
}

void inst_gt() {
  NOS = (NOS > TOS) ? -1 : 0;
  inst_drop();
}

void inst_fetch() {
  switch (TOS) {
    case -1: TOS = sp - 1; break;
    case -2: TOS = rp; break;
    case -3: TOS = IMAGE_SIZE; break;
    default: TOS = memory[TOS]; break;
  }
}

void inst_store() {
  if (TOS <= IMAGE_SIZE && TOS >= 0) {
    memory[TOS] = NOS;
    inst_drop();
    inst_drop();
  } else {
    ip = IMAGE_SIZE;
  }
}

void inst_add() {
  NOS += TOS;
  inst_drop();
}

void inst_sub() {
  NOS -= TOS;
  inst_drop();
}

void inst_mul() {
  NOS *= TOS;
  inst_drop();
}

void inst_divmod() {
  CELL a, b;
  a = TOS;
  b = NOS;
  TOS = b / a;
  NOS = b % a;
}

void inst_and() {
  NOS = TOS & NOS;
  inst_drop();
}

void inst_or() {
  NOS = TOS | NOS;
  inst_drop();
}

void inst_xor() {
  NOS = TOS ^ NOS;
  inst_drop();
}

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

void inst_zret() {
  if (TOS == 0) {
    inst_drop();
    ip = TORS;
    rp--;
  }
}

void inst_halt() {
  ip = IMAGE_SIZE;
}

void inst_ie() {
  sp++;
  TOS = NUM_DEVICES;
}

void inst_iq() {
  CELL Device = TOS;
  inst_drop();
  IO_queryHandlers[Device]();
}

void inst_ii() {
  CELL Device = TOS;
  inst_drop();
  IO_deviceHandlers[Device]();
}

Handler instructions[NUM_OPS] = {
  inst_nop, inst_lit, inst_dup, inst_drop, inst_swap, inst_push, inst_pop,
  inst_jump, inst_call, inst_ccall, inst_return, inst_eq, inst_neq, inst_lt,
  inst_gt, inst_fetch, inst_store, inst_add, inst_sub, inst_mul, inst_divmod,
  inst_and, inst_or, inst_xor, inst_shift, inst_zret, inst_halt, inst_ie,
  inst_iq, inst_ii
};

void ngaProcessOpcode(CELL opcode) {
  if (opcode != 0)
    instructions[opcode]();
}

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
