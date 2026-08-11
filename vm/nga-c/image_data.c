/* Compile the generated embedded image as its own translation unit. */

#include "retro.h"
#include "image.c"

V load_embedded_image(NgaState *vm) {
  int i;
  for (i = 0; i < ngaImageCells; i++)
    vm->memory[i] = ngaImage[i];
}
