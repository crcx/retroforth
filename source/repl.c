/*  ____   ____ ______ ____    ___
    || \\ ||    | || | || \\  // \\
    ||_// ||==    ||   ||_// ((   ))
    || \\ ||___   ||   || \\  \\_//
    a personal, minimalistic forth

    Going back to Retro 10, the `listener` has been the most
    common interface for Retro. This is a version of it for
    Retro 12.

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

int main(int argc, char **argv) {
  ngaPrepare();
  ngaLoadImage("ngaImage");
  update_rx();
  printf("RETRO 12 (rx-%d.%d)\n", memory[4] / 100, memory[4] % 100);
  char input[1024];
  printf("%d MAX, TIB @ %d, Heap @ %d\n\n", IMAGE_SIZE, TIB, Heap);
  while(1) {
    Dictionary = memory[2];
    read_token(stdin, input, 0);
    if (strcmp(input, "bye") == 0)
      exit(0);
    else
      evaluate(input);
  }
  exit(0);
}
