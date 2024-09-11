/* RETRO : a personal, minimalistic forth

  This interface layer will create a new binary that
  bundles the RETRO virtual machine and image file.

  The VM and image are embedded in this as ELF sections.
  This will extract them, compile code from a file into
  the image, then embed the image into the VM binary.

  Due to the way this works, it requires a Unix-like OS
  and the `objcopy` binary in the path.

  Copyright (c) 2016 - 2021, Charles Childers
*/


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
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
#define IMAGE_SIZE   524288       /* Amount of RAM, in cells           */
#endif

#ifndef ADDRESSES
#define ADDRESSES    256          /* Depth of address stack            */
#endif

#ifndef STACK_DEPTH
#define STACK_DEPTH  256          /* Depth of data stack               */
#endif

#define TIB memory[7]             /* Location of TIB                   */

#define D_OFFSET_LINK     0       /* Dictionary Format Info. Update if */
#define D_OFFSET_XT       1       /* you change the dictionary fields. */
#define D_OFFSET_CLASS    2
#define D_OFFSET_STACK    5
#define D_OFFSET_NAME     9

#define NUM_DEVICES       1       /* Set the number of I/O devices     */

CELL sp, rp, ip;
CELL data[STACK_DEPTH];
CELL address[ADDRESSES];
CELL memory[IMAGE_SIZE + 1];
#define TOS  data[sp]
#define NOS  data[sp-1]
#define TORS address[rp]


typedef void (*Handler)(void);

Handler IO_deviceHandlers[NUM_DEVICES + 1];
Handler IO_queryHandlers[NUM_DEVICES + 1];

CELL ngaLoadImage(char *imageFile);
void ngaPrepare();
void ngaProcessOpcode(CELL opcode);
void ngaProcessPackedOpcodes(CELL opcode);
int ngaValidatePackedOpcodes(CELL opcode);

CELL stack_pop();
void stack_push(CELL value);
int string_inject(char *str, int buffer);
char *string_extract(int at);
CELL d_xt_for(char *Name, CELL Dictionary);
void execute(int cell);
void evaluate(char *s);
void read_token(FILE *file, char *token_buffer, int echo);

/* This assumes some knowledge of the ngaImage format for the
   Retro language. If things change there, these will need to
   be adjusted to match. */

extern CELL Dictionary, Heap, Compiler;
extern CELL notfound;

void generic_output() {
  putc(stack_pop(), stdout);
  fflush(stdout);
}

void generic_output_query() {
  stack_push(0);
  stack_push(0);
}

void dump_stack() {
  CELL i;
  if (sp == 0)
    return;
  printf("\nStack: ");
  for (i = 1; i <= sp; i++) {
    if (i == sp)
      printf("[ TOS: %lld ]", (long long)data[i]);
    else
      printf("%lld ", (long long)data[i]);
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
    if (strcmp(source, "~~~") == 0) {
      if (inBlock == 0)
        inBlock = 1;
      else
        inBlock = 0;
    } else {
      if (inBlock == 1) {
        evaluate(source);
        putchar('.');
      }
    }
    tokens++;
  }
  fclose(fp);
  putchar('\n');
  return tokens;
}

void extract_runtime(char *src) {
  char buffer[4096];
  snprintf(buffer, 4096, "objcopy -O binary --only-section=.runtime --set-section-flags .runtime=alloc %s a.out", src);
  system(buffer);
}

void extract_image(char *src) {
  char buffer[4096];
  snprintf(buffer, 4096, "objcopy -O binary --only-section=.ngaImage --set-section-flags .ngaImage=alloc %s __ngaImage", src);
  system(buffer);
}

void generate_turnkey(void) {
  system("objcopy --add-section .ngaImage=__ngaImage --set-section-flags .ngaImage=noload,readonly a.out");
  chmod("a.out", 493);
}

int validate_image(int tokens) {
  if (tokens == 0) {
    unlink("a.out");
    printf("Error: no tokens in source file!\n");
    return -1;
  }
  if (memory[1] == 0 || memory[1] == -1) {
    unlink("a.out");
    printf("Error: entry point not set!\n");
    return -2;
  }
  return 0;
}

void setup() {
  IO_deviceHandlers[0] = generic_output;
  IO_queryHandlers[0] = generic_output_query;
}

void write_image() {
  FILE *fp;
  if ((fp = fopen("__ngaImage", "wb")) == NULL) {
    printf("Unable to save the ngaImage!\n");
    exit(2);
  }
  fwrite(&memory, sizeof(CELL), memory[3] + 1, fp);
  fclose(fp);
}

void patch_entry(CELL a) {
  memory[1] = a;
  printf("Entry set to %lld\n", (long long)a);
}

int main(int argc, char **argv) {
  int tokens;
  if (argc < 3) {
    printf("Missing arguments\n");
    exit(1);
  }

  ngaPrepare();
  extract_runtime(argv[0]);
  extract_image(argv[0]);
  ngaLoadImage("__ngaImage");
  tokens = include_file(argv[1]);
  patch_entry(d_xt_for(argv[2], Dictionary));
  write_image();
  generate_turnkey();
  unlink("__ngaImage");

  printf("\nFinal image is %lld cells\n", (long long)memory[3]);
  return validate_image(tokens);
}

CELL Dictionary, Heap, Compiler;
CELL notfound;

/* Some I/O Parameters */

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

int d_xt(CELL dt) {
  return dt + D_OFFSET_XT;
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
    opcode = memory[ip];
    if (ngaValidatePackedOpcodes(opcode) != 0) {
      ngaProcessPackedOpcodes(opcode);
    } else {
      printf("Invalid instruction!\n");
      exit(1);
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
  CELL interpret;
  if (strlen(s) == 0)
    return;
  update_rx();
  interpret = d_xt_for("interpret", Dictionary);
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
  int ch, count;
  ch = getc(file);
  count = 0;
  while (not_eol(ch))
  {
    if ((ch == 8 || ch == 127) && count > 0)
      count--;
    else
      token_buffer[count++] = ch;
    ch = getc(file);
  }
  token_buffer[count] = '\0';
}


/* Nga ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Copyright (c) 2008 - 2018, Charles Childers
   Copyright (c) 2009 - 2010, Luke Parrish
   Copyright (c) 2010,        Marc Simpson
   Copyright (c) 2010,        Jay Skeer
   Copyright (c) 2011,        Kenneth Keating
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifndef NUM_DEVICES
#define NUM_DEVICES 0
#endif

CELL ngaLoadImage(char *imageFile) {
  FILE *fp;
  CELL imageSize;
  long fileLen;
  CELL i;
  if ((fp = fopen(imageFile, "rb")) != NULL) {
    /* Determine length (in cells) */
    fseek(fp, 0, SEEK_END);
    fileLen = ftell(fp) / sizeof(CELL);
    rewind(fp);
    /* Read the file into memory */
    imageSize = fread(&memory, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
  return imageSize;
}

void ngaPrepare() {
  ip = sp = rp = 0;
  for (ip = 0; ip < IMAGE_SIZE; ip++)
    memory[ip] = 0; /* NOP */
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
    case -4: TOS = CELL_MIN; break;
    case -5: TOS = CELL_MAX; break;
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

Handler instructions[] = {
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
