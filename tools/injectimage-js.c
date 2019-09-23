/*  ____   ____ ______ ____    ___
    || \\ ||    | || | || \\  // \\
    ||_// ||==    ||   ||_// ((   ))
    || \\ ||___   ||   || \\  \\_//
    a personal, minimalistic forth

    This loads an image file and generates a C formatted
    output. It's used to create the `image.c` that gets
    linked into `rre`.

    Copyright (c) 2016-2019 Charles Childers
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

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
CELL memory[128 KiB];
char source[16 KiB];

void read_line(FILE *file, char *line_buffer) {
  int ch, count;
  if (file == NULL || line_buffer == NULL)
  {
    printf("Error: file or line buffer pointer is null.");
    exit(1);
  }
  ch = getc(file);
  count = 0;
  while ((ch != '\n') && (ch != EOF)) {
    line_buffer[count] = ch;
    count++;
    ch = getc(file);
  }
  line_buffer[count] = '\0';
}

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

void extract(char *fname, int32_t size) {
  char *buffer = (char *)source;
  FILE *fp;
  int32_t i;
  int32_t n;

  fp = fopen(fname, "r");
  if (fp == NULL)
    return;
  while (!feof(fp)) {
    read_line(fp, buffer);
    if (strcmp(buffer, "<<<IMAGE>>>") == 0) {
      i = 0;
      n = 0;
      while (i < size) {
        n++;
        if (n == 20) {
          printf("\n");
          n = 0;
        }
        if (i+1 < size)
          printf("%d,", memory[i]);
        else
          printf("%d\n", memory[i]);
        i++;
      }
    } else {
      printf("%s\n", buffer);
    }
  }
  fclose(fp);
}

int main(int argc, char **argv) {
  int32_t size = 0;

  if (argc == 2)
      size = ngaLoadImage(argv[1]);
  else
      size = ngaLoadImage("ngaImage");

  extract("source/interfaces/alternates/retro12.html", size);

  exit(0);
}
