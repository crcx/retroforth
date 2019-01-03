Muri is a minimalistic assembler for Nga.

The standard assembler for Nga is Naje. This is an attempt at
making a much smaller assembler at a cost of requiring more
manual knowledge of the Nga virtual machine and its encodings.

Input syntax

    <directive> <data>

Directives are a single character. Muri recognizes:

* **i** for instructions
* **d** for numeric data
* **c** for character data
* **s** for string data
* **:** for creating a label
* **r** for references to labels

Instructions are packed up to four instructions per location.
You can specify them using the first two characters of the
instruction name. For a non operation, use '..' instead of
'no'.

    0  nop      7  jump      14  gt        21  and     28  iquery
    1  lit <v>  8  call      15  fetch     22  or      29  iinteract
    2  dup      9  ccall     16  store     23  xor
    3  drop    10  return    17  add       24  shift
    4  swap    11  eq        18  sub       25  zret
    5  push    12  neq       19  mul       26  end
    6  pop     13  lt        20  divmod    27  ienum

E.g., for a sequence of dup, multiply, no-op, drop:

    i dupmu..dr

An example of a small program:

    i liju....
    r main
    : square
    i dumure..
    : main
    i lilica..
    d 12
    r square
    i en......

As mentioned earlier this requires knowledge of Nga architecture.
While you can pack up to four instructions per location, you
should not place anything after an instruction that modifies the
instruction pointer. These are: ju, ca, cc, re, and zr.

----

The code begins with the necessary C headers.

~~~
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
~~~

And then a couple of constants that determine overall memory
usage.

~~~
#define KiB * 1024
#define MAX_NAMES 1024
#define STRING_LEN 64
#define IMAGE_SIZE 128 KiB
~~~

Next, define the arrays for the reference handling.

~~~
char    Labels[MAX_NAMES][STRING_LEN];
int32_t Pointers[MAX_NAMES];
int32_t np;
~~~

And then the variables and array for the target memory and
source buffer:

~~~
char source[1 KiB];
int32_t target[IMAGE_SIZE];
int32_t here;
~~~

And that's the end of the data part. Now on to routines.

First up, something to save the generated image file.

~~~
void save() {
  FILE *fp;
  if ((fp = fopen("ngaImage", "wb")) == NULL) {
    printf("Unable to save the image!\n");
    exit(2);
  }
  fwrite(&target, sizeof(int32_t), here, fp);
  fclose(fp);
}
~~~

Next, functions related to the reference tables. We have two.
The `lookup()` searches the tables for a name and returns
either -1 (if not found) or the address that corresponds to it.

~~~
int32_t lookup(char *name) {
  int32_t slice = -1;
  int32_t n = np;
  while (n > 0) {
    n--;
    if (strcmp(Labels[n], name) == 0)
      slice = Pointers[n];
  }
  return slice;
}
~~~

The second, `add_label()` handles adding a new label to the
table. It also terminates the build if the label already exists.

~~~
void add_label(char *name, int32_t slice) {
  if (lookup(name) == -1) {
    strcpy(Labels[np], name);
    Pointers[np] = slice;
    np++;
  } else {
    printf("Fatal error: %s already defined\n", name);
    exit(0);
  }
}
~~~

This next routine reads a line from a file into the input buffer.

~~~
void read_line(FILE *file, char *line_buffer) {
  int ch = getc(file);
  int count = 0;
  while ((ch != '\n') && (ch != EOF)) {
    line_buffer[count] = ch;
    count++;
    ch = getc(file);
  }
  line_buffer[count] = '\0';
}
~~~

This one is a little messy. It just checks a source string
against the list of instructions and returns the corresponding
opcode. It returns 0 (nop) for anything unrecognized.

~~~
int32_t opcode_for(char *s) {
  if (strcmp(s, "..") == 0) return 0;  if (strcmp(s, "li") == 0) return 1;
  if (strcmp(s, "du") == 0) return 2;  if (strcmp(s, "dr") == 0) return 3;
  if (strcmp(s, "sw") == 0) return 4;  if (strcmp(s, "pu") == 0) return 5;
  if (strcmp(s, "po") == 0) return 6;  if (strcmp(s, "ju") == 0) return 7;
  if (strcmp(s, "ca") == 0) return 8;  if (strcmp(s, "cc") == 0) return 9;
  if (strcmp(s, "re") == 0) return 10; if (strcmp(s, "eq") == 0) return 11;
  if (strcmp(s, "ne") == 0) return 12; if (strcmp(s, "lt") == 0) return 13;
  if (strcmp(s, "gt") == 0) return 14; if (strcmp(s, "fe") == 0) return 15;
  if (strcmp(s, "st") == 0) return 16; if (strcmp(s, "ad") == 0) return 17;
  if (strcmp(s, "su") == 0) return 18; if (strcmp(s, "mu") == 0) return 19;
  if (strcmp(s, "di") == 0) return 20; if (strcmp(s, "an") == 0) return 21;
  if (strcmp(s, "or") == 0) return 22; if (strcmp(s, "xo") == 0) return 23;
  if (strcmp(s, "sh") == 0) return 24; if (strcmp(s, "zr") == 0) return 25;
  if (strcmp(s, "en") == 0) return 26; if (strcmp(s, "ie") == 0) return 27;
  if (strcmp(s, "iq") == 0) return 28; if (strcmp(s, "ii") == 0) return 29;
  return 0;
}
~~~

Now for the first pass. This lays down code, with dummy values
for the references. They will be resolved in `pass2()`.

~~~
void pass1(char *fname) {
  int inBlock = 0;
  char *buffer = (char *)source;
  unsigned int opcode;
  char inst[3];
  FILE *fp;
  inst[2] = '\0';
  here = 0;
  fp = fopen(fname, "r");
  if (fp == NULL) {
    printf("Unable to load file\n");
    exit(2);
  }
  while (!feof(fp)) {
    read_line(fp, buffer);
    if (strcmp(buffer, "~~~") == 0) {
      if (inBlock == 0)
        inBlock = 1;
      else
        inBlock = 0;
    } else {
      if (inBlock == 1) {
        if (buffer[1] == '\t' || buffer[1] == ' ') {
          switch (buffer[0]) {
            case 'i': memcpy(inst, buffer + 8, 2);
                      opcode = opcode_for(inst);
                      opcode = opcode << 8;
                      memcpy(inst, buffer + 6, 2);
                      opcode += opcode_for(inst);
                      opcode = opcode << 8;
                      memcpy(inst, buffer + 4, 2);
                      opcode += opcode_for(inst);
                      opcode = opcode << 8;
                      memcpy(inst, buffer + 2, 2);
                      opcode += opcode_for(inst);
                      target[here++] = opcode;
                      break;
            case 'r': target[here++] = -1;
                      break;
            case 'd': target[here++] = atoi(buffer+2);
                      break;
            case 'c': target[here++] = buffer[2];
                      break;
            case 's': opcode = 2;
                      while (opcode < strlen(buffer))
                        target[here++] = buffer[opcode++];
                      target[here++] = 0;
                      break;
            case ':': add_label(buffer+2, here);
                      break;
          }
        }
      }
    }
  }
  fclose(fp);
}
~~~

The second pass skips over any instructions or data, but replaces
the dummy values for each reference with the actual address
(recorded as part of `pass1()`).

~~~
void pass2(char *fname) {
  char *buffer;
  FILE *fp;
  int inBlock;
  inBlock = 0;
  buffer = (char *)source;
  here = 0;
  fp = fopen(fname, "r");
  while (!feof(fp)) {
    read_line(fp, buffer);
    if (strcmp(buffer, "~~~") == 0) {
      if (inBlock == 0)
        inBlock = 1;
      else
        inBlock = 0;
    } else {
      if (inBlock == 1) {
        switch (buffer[0]) {
          case 'i': here++;                             break;
          case 'r': target[here++] = lookup(buffer+2);
                    if (lookup(buffer+2) == -1)
                      printf("Lookup failed: '%s'\n", buffer+2);
                                                        break;
          case 'd': here++;                             break;
          case 'c': here++;                             break;
          case 's': here = here + strlen(buffer) - 1;   break;
          case ':':                                     break;
        }
      }
    }
  }
  fclose(fp);
}
~~~

And then the top level wrapper.

~~~
int main(int argc, char **argv) {
  np = 0;
  if (argc > 1) {
    pass1(argv[1]);
    pass2(argv[1]);
    save();
    printf("Wrote %d cells to ngaImage\n", here);
  }
  else
    printf("muri\n(c) 2017-2019 charles childers\n\n%s filename\n", argv[0]);
  return 0;
}
~~~
