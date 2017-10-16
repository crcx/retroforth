/*  ____   ____ ______ ____    ___
    || \\ ||    | || | || \\  // \\
    ||_// ||==    ||   ||_// ((   ))
    || \\ ||___   ||   || \\  \\_//
    a personal, minimalistic forth

    This is `rre`, short for `run retro and exit`. It's the
    basic interface layer for Retro on Linux and macOS.

    `rre` embeds the image file into the binary, so the
    compiled version of this is all you need to have a
    functional system.

    Copyright (c) 2016, 2017 Charles Childers
*/

//LIBS m
//USES nga
//USES bridge
//USES image
//HEAD config
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "nga.h"
#include "bridge.h"

#include "io/posix_files.c"
#include "io/getc.c"

#ifdef FPU
#include "io/fpu.c"
#endif

#ifdef ARGV
#include "io/posix_args.c"
#endif

#ifdef GOPHER
#include "io/gopher.c"
#endif

/* Compile image.c and link against the image.o */
extern CELL ngaImageCells;
extern CELL ngaImage[];

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


int fenced(char *s)
{
  int a = strcmp(s, "```");
  int b = strcmp(s, "~~~");
  if (a == 0) return 1;
  if (b == 0) return 1;
              return 0;
}


void include_file(char *fname) {
  int inBlock = 0;
  char source[64000];
  char fence[4];
  FILE *fp;
  fp = fopen(fname, "r");
  if (fp == NULL)
    return;
  while (!feof(fp))
  {
    read_token(fp, source, 0);
    strncpy(fence, source, 3);
    fence[3] = '\0';
    if (fenced(fence)) {
      if (inBlock == 0)
        inBlock = 1;
      else
        inBlock = 0;
    } else {
      if (inBlock == 1)
        evaluate(source);
    }
  }
  fclose(fp);
}


void evaluate_string(char *s) {
  char source[64000];
  char *x = s;
  while (x < strlen(s) + s) {
    x = read_token_str(x, (char *)source, 0);
    evaluate(source);
  }
}

int main(int argc, char **argv) {
  int i;
  ngaPrepare();
  for (i = 0; i < ngaImageCells; i++)
    memory[i] = ngaImage[i];
  update_rx();

#ifdef ARGV
  sys_argc = argc;
  sys_argv = argv;
  evaluate_string(posix_args);
#endif

  evaluate_string(posix_files);
  evaluate_string(posix_getc);

#ifdef FPU
  evaluate_string(fpu);
#endif

#ifdef GOPHER
  evaluate_string(gopher);
#endif

  include_file(argv[1]);

  if (sp >= 1)
    dump_stack();

  exit(0);
}

