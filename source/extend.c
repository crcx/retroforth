/*  ____   ____ ______ ____    ___
    || \\ ||    | || | || \\  // \\
    ||_// ||==    ||   ||_// ((   ))
    || \\ ||___   ||   || \\  \\_//
    a personal, minimalistic forth

    This is a quick interface layer that loads and runs a
    source file, then saves a new image file. It's used to
    merge the `retro.forth` into the base `rx` image.

    Copyright (c) 2016, 2017 Charles Childers
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define CELL         int32_t
#define IMAGE_SIZE   524288 * 16
#define ADDRESSES    2048
#define STACK_DEPTH  512

enum vm_opcode {
  VM_NOP,  VM_LIT,    VM_DUP,   VM_DROP,    VM_SWAP,   VM_PUSH,  VM_POP,
  VM_JUMP, VM_CALL,   VM_CCALL, VM_RETURN,  VM_EQ,     VM_NEQ,   VM_LT,
  VM_GT,   VM_FETCH,  VM_STORE, VM_ADD,     VM_SUB,    VM_MUL,   VM_DIVMOD,
  VM_AND,  VM_OR,     VM_XOR,   VM_SHIFT,   VM_ZRET,   VM_END
};
#define NUM_OPS VM_END + 1

CELL sp, rp, ip;
CELL data[STACK_DEPTH];
CELL address[ADDRESSES];
CELL memory[IMAGE_SIZE + 1];
#define TOS  data[sp]
#define NOS  data[sp-1]
#define TORS address[rp]

CELL ngaLoadImage(char *imageFile);
void ngaPrepare();
void ngaProcessOpcode(CELL opcode);
void ngaProcessPackedOpcodes(int opcode);
int ngaValidatePackedOpcodes(CELL opcode);

/* This assumes some knowledge of the ngaImage format for the
   Retro language. If things change there, these will need to
   be adjusted to match. */

#define TIB            1025
#define D_OFFSET_LINK     0
#define D_OFFSET_XT       1
#define D_OFFSET_CLASS    2
#define D_OFFSET_NAME     3

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

void dump_stack() {
  CELL i;
  if (sp == 0)
    return;
  printf("\nStack: ");
  for (i = 1; i <= sp; i++) {
    if (i == sp)
      printf("[ TOS: %d ]", data[i]);
    else
      printf("%d ", data[i]);
  }
  printf("\n");
}

int include_file(char *fname) {
  FILE *fp;
  char source[64000];
  int inBlock = 0;
  int tokens = 0;
  fp = fopen(fname, "r");
  if (fp == NULL)
    return 0;
  while (!feof(fp))
  {
    read_token(fp, source, 0);
    if (strcmp(source, "````") == 0) {
      if (inBlock == 0)
        inBlock = 1;
      else
        inBlock = 0;
    } else {
      if (inBlock == 1) {
        evaluate(source);
      }
    }
    tokens++;
  }
  fclose(fp);
  return tokens;
}

void stats() {
  update_rx();
  printf("  Heap @ %d\n", Heap);
}

int main(int argc, char **argv) {
  printf("RETRO12\n");
  printf("+ initialize\n");
  ngaPrepare();
  printf("+ load image\n");
  ngaLoadImage("ngaImage");
  stats();
  dump_stack();
  printf("+ load %s\n", argv[1]);
  int tokens = include_file(argv[1]);
  printf("  processed %d tokens\n", tokens);
  stats();
  printf("+ save new image\n");
  FILE *fp;
  if ((fp = fopen("ngaImage", "wb")) == NULL) {
    printf("Unable to save the ngaImage!\n");
    exit(2);
  }
  fwrite(&memory, sizeof(CELL), memory[3] + 1, fp);
  fclose(fp);
  return 0;
}

CELL Dictionary, Heap, Compiler;
CELL notfound;

/* Some I/O Parameters */

#define IO_TTY_PUTC  1000

CELL stack_pop() {
  sp--;
  return data[sp + 1];
}

void stack_push(CELL value) {
  sp++;
  data[sp] = value;
}

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
  CELL a, b;
  CELL opcode;
  rp = 1;
  ip = cell;
  while (ip < IMAGE_SIZE) {
    if (ip == notfound) {
      printf("%s ?\n", string_extract(TIB));
    }
    opcode = memory[ip];
    if (ngaValidatePackedOpcodes(opcode) != 0) {
      ngaProcessPackedOpcodes(opcode);
    } else if (opcode >= 0 && opcode < 27) {
      ngaProcessOpcode(opcode);
    } else {
      switch (opcode) {
        case IO_TTY_PUTC:  putc(stack_pop(), stdout); fflush(stdout); break;
        default:   printf("Invalid instruction!\n");
                   printf("At %d, opcode %d\n", ip, opcode);
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

/* Nga ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Copyright (c) 2008 - 2017, Charles Childers
   Copyright (c) 2009 - 2010, Luke Parrish
   Copyright (c) 2010,        Marc Simpson
   Copyright (c) 2010,        Jay Skeer
   Copyright (c) 2011,        Kenneth Keating
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

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
  int a;
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
  int a, b;
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
  memory[TOS] = NOS;
  inst_drop();
  inst_drop();
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
  int a, b;
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

void inst_end() {
  ip = IMAGE_SIZE;
}

typedef void (*Handler)(void);

Handler instructions[NUM_OPS] = {
  inst_nop, inst_lit, inst_dup, inst_drop, inst_swap, inst_push, inst_pop,
  inst_jump, inst_call, inst_ccall, inst_return, inst_eq, inst_neq, inst_lt,
  inst_gt, inst_fetch, inst_store, inst_add, inst_sub, inst_mul, inst_divmod,
  inst_and, inst_or, inst_xor, inst_shift, inst_zret, inst_end
};

void ngaProcessOpcode(CELL opcode) {
  instructions[opcode]();
}

int ngaValidatePackedOpcodes(CELL opcode) {
  CELL raw = opcode;
  CELL current;
  int valid = -1;
  int i;
  for (i = 0; i < 4; i++) {
    current = raw & 0xFF;
    if (!(current >= 0 && current <= 26))
      valid = 0;
    raw = raw >> 8;
  }
  return valid;
}

void ngaProcessPackedOpcodes(int opcode) {
  CELL raw = opcode;
  int i;
  for (i = 0; i < 4; i++) {
    ngaProcessOpcode(raw & 0xFF);
    raw = raw >> 8;
  }
}
