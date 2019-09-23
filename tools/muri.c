#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "strl.h"

#ifndef BIT64
#define CELL int32_t
#define CELL_MIN INT_MIN + 1
#define CELL_MAX INT_MAX - 1
#else
#define CELL int64_t
#define CELL_MIN LLONG_MIN + 1
#define CELL_MAX LLONG_MAX - 1
#endif

#define KiB * 1024
#define MAX_NAMES 1024
#define STRING_LEN 64
#define IMAGE_SIZE 128 KiB

char Labels[MAX_NAMES][STRING_LEN];
CELL Pointers[MAX_NAMES];
CELL np;
char source[1 KiB];
CELL target[IMAGE_SIZE];
CELL here;

void save() {
  FILE *fp;
  if ((fp = fopen("ngaImage", "wb")) == NULL) {
    printf("Unable to save the image!\n");
    exit(2);
  }
  fwrite(&target, sizeof(CELL), here, fp);
  fclose(fp);
}

CELL lookup(char *name) {
  CELL slice = -1;
  CELL n = np;
  while (n > 0) {
    n--;
    if (strcmp(Labels[n], name) == 0)
      slice = Pointers[n];
  }
  return slice;
}

void add_label(char *name, CELL slice) {
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

CELL opcode_for(char *s) {
  char* opcodeList = "..lidudrswpupojucaccreeqneltgtfestadsumudianorxoshzrenieiqii";
  int16_t* s16 = (int16_t *)s;
  int16_t* op16 = (int16_t *)opcodeList;
  int i = 0;
  for(i = 0; i <= 30; i++){
    if(s16[0] == op16[i]){
      return i;
    }
  }
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
    printf("Wrote %lld cells to ngaImage\n", (long long)here);
  }
  else
    printf("muri\n(c) 2017-2019 charles childers\n\n%s filename\n", argv[0]);
  return 0;
}
