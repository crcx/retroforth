/*
 * This demonstrates calling RETRO code from C.
 *
 * Assuming this is named `demo.c`:
 *
 * (1) Prepare a demo image:
 *
 *     cp ngaImage demoImage
 *     retro-extend demoImage demo.c
 *
 * (2) Build:
 *
 *     make demo
 *
 * (3) Run:
 *
 *     ./demo
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * The next few lines will be compiled into demoImage
 * when the `retro-extend` line is run from step 1.
 *
~~~
:average (...n-m) [ [ + ] times ] sip / ;
:hello 'hello_world! s:put nl ;
~~~
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#define CELL         int32_t
#define IMAGE_SIZE   242000
#define STACK_DEPTH  256
#define NUM_DEVICES  2

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TIB            1025       /* Location of TIB                   */

#define D_OFFSET_LINK     0       /* Dictionary Format Info. Update if */
#define D_OFFSET_XT       1       /* you change the dictionary fields. */
#define D_OFFSET_CLASS    2
#define D_OFFSET_NAME     3

CELL sp, rp, ip;
CELL data[STACK_DEPTH];
CELL address[STACK_DEPTH];
CELL memory[IMAGE_SIZE + 1];
void ngaLoadImage();
void ngaPrepare();
void ngaProcessOpcode(CELL opcode);
void ngaProcessPackedOpcodes(CELL opcode);
int ngaValidatePackedOpcodes(CELL opcode);

char string_data[8192];

/*---------------------------------------------------------------------
  Strings are next. RETRO uses C-style NULL terminated strings. So I
  can easily inject or extract a string. Injection iterates over the
  string, copying it into the image. This also takes care to ensure
  that the NULL terminator is added.
  ---------------------------------------------------------------------*/

CELL string_inject(char *str, CELL buffer) {
  CELL m, i;
  if (!str) {
    memory[buffer] = 0;
    return 0;
  }
  m = strlen(str);
  i = 0;
  while (m > 0) {
    memory[buffer + i] = (CELL)str[i];
    memory[buffer + i + 1] = 0;
    m--; i++;
  }
  return buffer;
}


/*---------------------------------------------------------------------
  Extracting a string is similar, but I have to iterate over the VM
  memory instead of a C string and copy the charaters into a buffer.
  This uses a static buffer (`string_data`) as I prefer to avoid using
  `malloc()`.
  ---------------------------------------------------------------------*/

char *string_extract(CELL at) {
  CELL starting = at;
  CELL i = 0;
  while(memory[starting] && i < 8192)
    string_data[i++] = (char)memory[starting++];
  string_data[i] = 0;
  return (char *)string_data;
}


/*---------------------------------------------------------------------
  Continuing along, I now define functions to access the dictionary.

  RETRO's dictionary is a linked list. Each entry is setup like:

  0000  Link to previous entry (NULL if this is the root entry)
  0001  Pointer to definition start
  0002  Pointer to class handler
  0003  Start of a NULL terminated string with the word name

  First, functions to access each field. The offsets were defineed at
  the start of the file.
  ---------------------------------------------------------------------*/

CELL d_link(CELL dt) {
  return dt + D_OFFSET_LINK;
}

CELL d_xt(CELL dt) {
  return dt + D_OFFSET_XT;
}

CELL d_class(CELL dt) {
  return dt + D_OFFSET_CLASS;
}

CELL d_name(CELL dt) {
  return dt + D_OFFSET_NAME;
}


/*---------------------------------------------------------------------
  Next, a more complext word. This will walk through the entries to
  find one with a name that matches the specified name. This is *slow*,
  but works ok unless you have a really large dictionary. (I've not
  run into issues with this in practice).
  ---------------------------------------------------------------------*/

CELL d_lookup(CELL Dictionary, char *name) {
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


/*---------------------------------------------------------------------
  My last dictionary related word returns the `xt` pointer for a word.
  This is used to help keep various important bits up to date.
  ---------------------------------------------------------------------*/

CELL d_xt_for(char *Name, CELL Dictionary) {
  return memory[d_xt(d_lookup(Dictionary, Name))];
}


CELL stack_pop() {
  sp--;
  return data[sp + 1];
}

void stack_push(CELL value) {
  sp++;
  data[sp] = value;
}

void output() {
  putc(stack_pop(), stdout);
}

void output_query() {
  stack_push(0);
  stack_push(0);
}

void input() {
  stack_push(getc(stdin));
  if (data[sp] == 127) data[sp] = 8;
}

void input_query() {
  stack_push(0);
  stack_push(1);
}

void execute(CELL cell) {
  rp = 1;
  ip = cell;
  while (ip < IMAGE_SIZE) {
    if (ngaValidatePackedOpcodes(memory[ip]) != 0) {
      ngaProcessPackedOpcodes(memory[ip]);
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
  ngaPrepare();
  ngaLoadImage("demoImage");
  execute(d_xt_for("hello", memory[2]));
}

typedef void (*Handler)(void);
Handler IO_deviceHandlers[] = { output, input };
Handler IO_queryHandlers[] = { output_query, input_query };

void ngaLoadImage(char *imageFile) {
  FILE *fp;
  CELL i;
  long fileLen;
  if ((fp = fopen(imageFile, "rb")) != NULL) {
    fseek(fp, 0, SEEK_END);
    fileLen = ftell(fp) / sizeof(CELL);
    rewind(fp);
    fread(&memory, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
}

void ngaPrepare() {
  for (ip = 0; ip < IMAGE_SIZE; ip++)
    memory[ip] = 0;
  for (ip = 0; ip < STACK_DEPTH; ip++)
    data[ip] = address[ip] = 0;
  ip = sp = rp = 0;
}

void inst_nop() {
}

void inst_lit() {
  ip++;
  stack_push(memory[ip]);
}

void inst_dup() {
  sp++;
  data[sp] = data[sp-1];
}

void inst_drop() {
  data[sp] = 0;
  sp--;
}

void inst_swap() {
  CELL a;
  a = data[sp];
  data[sp] = data[sp-1];
  data[sp-1] = a;
}

void inst_push() {
  rp++;
  address[rp] = stack_pop();
}

void inst_pop() {
  stack_push(address[rp]);
  rp--;
}

void inst_jump() {
  ip = stack_pop() - 1;
}

void inst_call() {
  rp++;
  address[rp] = ip;
  ip = stack_pop() - 1;
}

void inst_ccall() {
  if (data[sp-1] != 0) {
    rp++;
    address[rp] = ip;
    ip = data[sp] - 1;
  }
  sp -= 2;
}

void inst_return() {
  ip = address[rp];
  rp--;
}

void inst_eq() {
  data[sp-1] = (data[sp-1] == data[sp]) ? -1 : 0;
  inst_drop();
}

void inst_neq() {
  data[sp-1] = (data[sp-1] != data[sp]) ? -1 : 0;
  inst_drop();
}

void inst_lt() {
  data[sp-1] = (data[sp-1] < data[sp]) ? -1 : 0;
  inst_drop();
}

void inst_gt() {
  data[sp-1] = (data[sp-1] > data[sp]) ? -1 : 0;
  inst_drop();
}

void inst_fetch() {
  switch (data[sp]) {
    case -1: data[sp] = sp - 1; break;
    case -2: data[sp] = rp; break;
    case -3: data[sp] = IMAGE_SIZE; break;
    default: data[sp] = memory[data[sp]]; break;
  }
}

void inst_store() {
  memory[data[sp]] = data[sp-1];
  sp -= 2;
}

void inst_add() {
  data[sp-1] += data[sp];
  inst_drop();
}

void inst_sub() {
  data[sp-1] -= data[sp];
  inst_drop();
}

void inst_mul() {
  data[sp-1] *= data[sp];
  inst_drop();
}

void inst_divmod() {
  CELL a, b;
  a = data[sp];
  b = data[sp-1];
  data[sp] = b / a;
  data[sp-1] = b % a;
}

void inst_and() {
  data[sp-1] = data[sp] & data[sp-1];
  inst_drop();
}

void inst_or() {
  data[sp-1] = data[sp] | data[sp-1];
  inst_drop();
}

void inst_xor() {
  data[sp-1] = data[sp] ^ data[sp-1];
  inst_drop();
}

void inst_shift() {
  CELL y = data[sp];
  CELL x = data[sp-1];
  if (data[sp] < 0)
    data[sp-1] = data[sp-1] << (data[sp] * -1);
  else {
    if (x < 0 && y > 0)
      data[sp-1] = x >> y | ~(~0U >> y);
    else
      data[sp-1] = x >> y;
  }
  inst_drop();
}

void inst_zret() {
  if (data[sp] == 0) {
    inst_drop();
    ip = address[rp];
    rp--;
  }
}

void inst_end() {
  ip = IMAGE_SIZE;
}

void inst_ie() {
  stack_push(NUM_DEVICES);
}

void inst_iq() {
  IO_queryHandlers[data[sp--]]();
}

void inst_ii() {
  IO_deviceHandlers[data[sp--]]();
}

Handler instructions[] = {
  inst_nop, inst_lit, inst_dup, inst_drop, inst_swap, inst_push, inst_pop,
  inst_jump, inst_call, inst_ccall, inst_return, inst_eq, inst_neq, inst_lt,
  inst_gt, inst_fetch, inst_store, inst_add, inst_sub, inst_mul, inst_divmod,
  inst_and, inst_or, inst_xor, inst_shift, inst_zret, inst_end, inst_ie,
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
  for (int i = 0; i < 4; i++) {
    current = raw & 0xFF;
    if (!(current >= 0 && current <= 29))
      valid = 0;
    raw = raw >> 8;
  }
  return valid;
}

void ngaProcessPackedOpcodes(CELL opcode) {
  CELL raw = opcode;
  for (int i = 0; i < 4; i++) {
    ngaProcessOpcode(raw & 0xFF);
    raw = raw >> 8;
  }
}
