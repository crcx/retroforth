/*  ____   ____ ______ ____    ___
    || \\ ||    | || | || \\  // \\
    ||_// ||==    ||   ||_// ((   ))
    || \\ ||___   ||   || \\  \\_//
    a personal, minimalistic forth

    This loads an image file and generates a C formatted
    output. It's used to create the `image.c` that gets
    linked into `rre`.

    Copyright (c) 2016, 2017 Charles Childers
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define CELL         int32_t
CELL memory[128*1024];

CELL ngaLoadImage(char *imageFile) {
  FILE *fp;
  CELL imageSize;
  long fileLen;
  if ((fp = fopen(imageFile, "rb")) != NULL) {
    /* Determine length (in cells) */
    fseek(fp, 0, SEEK_END);
    fileLen = ftell(fp) / sizeof(CELL);
    rewind(fp);
    /* Read the file into memory */
    imageSize = fread(&memory, sizeof(CELL), fileLen, fp);
    fclose(fp);
  }
  else {
    printf("Unable to find the ngaImage!\n");
    exit(1);
  }
  return imageSize;
}

void output_header(int size) {
  printf("#include <stdint.h>\n");
  printf("int32_t ngaImageCells = %d;\n", size);
  printf("int32_t ngaImage[] = { ");
}

int main(int argc, char **argv) {
  int32_t size = 0;
  int32_t i;
  if (argc == 2)
      size = ngaLoadImage(argv[1]);
  else
      size = ngaLoadImage("ngaImage");

  output_header(size);

  i = 0;
  while (i < size) {
    if (i+1 < size)
      printf("%d,", memory[i]);
    else
      printf("%d };\n", memory[i]);
    i++;
  }
  exit(0);
}
