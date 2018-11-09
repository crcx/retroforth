#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define KiB * 1024
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
char source[16 KiB];
int fenced(char *s)
{
  int a = strcmp(s, "```");
  int b = strcmp(s, "~~~");
  if (a == 0) return 1;
  if (b == 0) return 1;
              return 0;
}
void extract(char *fname) {
  char *buffer = (char *)source;
  char fence[4];
  FILE *fp;
  int inBlock;
  inBlock = 0;
  fp = fopen(fname, "r");
  if (fp == NULL)
    return;
  while (!feof(fp)) {
    read_line(fp, buffer);
    strncpy(fence, buffer, 3);
    fence[3] = '\0';
    if (fenced(fence)) {
      if (inBlock == 0)
        inBlock = 1;
      else
        inBlock = 0;
    } else {
      if ((inBlock == 1) && (strlen(buffer) != 0))
        printf("%s\n", buffer);
    }
  }
  fclose(fp);
}
int main(int argc, char **argv) {
  int i = 1;
  if (argc > 1) {
    while (i < argc) {
      extract(argv[i++]);
    }
  }
  else
    printf("unu\n(c) 2013-2018 charles childers\n\nTry:\n  %s filename\n", argv[0]);
  return 0;
}
