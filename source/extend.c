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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "nga.h"
#include "bridge.h"

//USES nga
//USES bridge

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
//        printf("%s ", source);
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
