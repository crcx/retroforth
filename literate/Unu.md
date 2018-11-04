# Unu

    unu
    (verb) (-hia) pull out, withdraw, draw out, extract.

Unu is a tool for extracting fenced code blocks from Markdown documents.

I always found documenting my projects to be annoying. Eventually I decided to start mixing the code into the commentary using Markdown. Unu is the tool I use to extract the sources from the original files. I've found that this makes it easier for me to keep the commentary up to date.

## The Code

### Headers

~~~
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
~~~

I use this for readability purposes.

~~~
#define KiB * 1024
~~~

The `read_line` function is intended to read in a single line
into a target buffer. This considers a line to end with either
a `\n` or EOF.

~~~
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
~~~

The line buffer needs to be big enough for the longest lines in
your source files. Here it's set to 16KiB, which suffices for
everything I've used Unu with so far.

~~~
char source[16 KiB];
~~~

Unu looks for Markdown style fenced blocks of code. It supports
both backticks and tildes for this. This will return `1` if the
line appears to be a start/stop of a fence, or `0` otherwise.

~~~
int fenced(char *s)
{
  int a = strcmp(s, "```");
  int b = strcmp(s, "~~~");
  if (a == 0) return 1;
  if (b == 0) return 1;
              return 0;
}
~~~

The actual `extract` function is straightforward.

* open the file
* read each line
  * if in a fenced region, write the lines to stdout
* close the file

~~~
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
~~~

And finally, the main routine, which just runs `extract` on
each specified file.

~~~
int main(int argc, char **argv) {
  int i = 1;
  if (argc > 1) {
    while (i < argc) {
      extract(argv[i++]);
    }
  }
  else
    printf("unu\n(c) 2013-2017 charles childers\n\nTry:\n  %s filename\n", argv[0]);
  return 0;
}
~~~
