#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "strl.h"
#define KiB * 1024
#define MAX_NAMES 1024
#define STRING_LEN 64
#define IMAGE_SIZE 128 KiB
char    Labels[MAX_NAMES][STRING_LEN];
int32_t Pointers[MAX_NAMES];
int32_t np;
char source[1 KiB];
int32_t target[IMAGE_SIZE];
int32_t here;
void save() {
  FILE *fp;
  if ((fp = fopen("ngaImage", "wb")) == NULL) {
    printf("Unable to save the image!\n");
    exit(2);
  }
  fwrite(&target, sizeof(int32_t), here, fp);
  fclose(fp);
}
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
void add_label(char *name, int32_t slice) {
  if (lookup(name) == -1) {
    bsd_strlcpy(Labels[np], name, STRING_LEN);
    Pointers[np] = slice;
    np++;
  } else {
    printf("Fatal error: %s already defined\n", name);
    exit(0);
  }
}
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
void pass2(char *fname) {
  char *buffer;
  FILE *fp;
  int inBlock;
  inBlock = 0;
  buffer = (char *)source;
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
        switch (buffer[0]) {
          case 'i': here++;                             break;
          case 'r': target[here++] = lookup(buffer+2);
                    if (lookup(buffer+2) == -1)
                      printf("Lookup failed: '%s'\n", buffer+2);
                                                        break;
          case 'd': here++;                             break;
          case 's': here = here + strlen(buffer) - 1;   break;
          case ':':                                     break;
        }
      }
    }
  }
  fclose(fp);
}
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
