/*  ____   ____ ______ ____    ___
    || \\ ||    | || | || \\  // \\
    ||_// ||==    ||   ||_// ((   ))
    || \\ ||___   ||   || \\  \\_//
    a personal, minimalistic forth

    This converts a text file into a C character array.

    Copyright (c) 2016, 2017 Charles Childers
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void include_file(char *fname) {
  int ch;
  FILE *fp;
  fp = fopen(fname, "r");
  if (fp == NULL)
    return;
  while (!feof(fp)) {
    ch = getc(fp);
    printf("%d, ", ch);
  }
  fclose(fp);
}


int main(int argc, char **argv) {
  printf("char %s[] = {", argv[1]);
  include_file("/dev/stdin");
  printf("0 };\n");
}
