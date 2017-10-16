/******************************************************
 * build
 *
 * This is a "make" replacement that determines
 * dependencies by looking at lines in a program. It
 * tries to find lines beginning with:
 *
 * //LIBS
 * //USES
 * //FLAGS
 *
 * It also looks at #include directives. Subdependencies
 * will be built first as necessary. You can nest the
 * //USES directives.
 *
 * Based on cmake.c by Tom Novelli
 ******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#define GCC_COMPILE "clang -c %s %s"

void    err(char *s, char *s2);
void    add(char *dst, char *src);
time_t  filedate(char *fn);
int     dep(char *fn, time_t t);
int     build(char *fn, time_t t);
void    make(char *fn);
int     main(int argc, char **argv);


/******************************************************
 *|F| Variables:
 *|F|   char obs[1024];
 *|F|   Hold the list of object files to build
 *|F|
 *|F|   char libs[1024];
 *|F|   Hold the list of libraries needed
 *|F|
 *|F|   char flags[1024];
 *|F|   Custom compiler flags
 *|F|
 ******************************************************/
char objs[1024];
char libs[1024];
char flags[1024];

/******************************************************
 *|F| void err(char *s, char *s2)
 *|F| Report errors and exit rebuild
 *|F|
 ******************************************************/
void err(char *s, char *s2)
{
  printf("ERROR: ");
  printf(s, s2);
  printf("\n");
  exit(2);
}


/******************************************************
 *|F| void add(char *dst, char *src)
 *|F| Append src to dst, if it's not already in the list.
 *|F|
 ******************************************************/
void add(char *dst, char *src)
{
  char *p = dst;
  int l = strlen(src);

  /* Search for src in dst (whole word) */
  while(*p) {
    if (!strncmp(p, src, l) && !p[l])
      return;
    for (; *p && !isspace(*p); p++);   /* skip word */
    for (; *p && isspace(*p); p++);    /* skip blanks */
  }

  /* If not found, append src to dst */
  *p++ = ' ';
  strcpy(p, src);
}


/******************************************************
 *|F| time_t filedate(char *fn)
 *|F|
 ******************************************************/
time_t filedate(char *fn)
{
  struct stat buf;
  if (-1 == stat(fn, &buf))
    return 0;
  return buf.st_mtime;
}


/******************************************************
 *|F| int dep(char *fn, time_t t)
 *|F| Check the dependencies in a .c or .h file
 *|F|
 ******************************************************/
int dep(char *fn, time_t t)
{
  char buf[256], *p;
  char s[256], *q;
  FILE *f;
  int result = 0;

  f = fopen(fn, "r");
  if (!f)
    err("Can't open '%s'", fn);
  while(fgets(buf, 256, f))
  {
    if (!strncasecmp(buf, "//LIBS", 6))
      for(p = buf + 6; *p;)
      {
        for(; isspace(*p); p++);
        for(q = p; !isspace(*q); q++);
        *q++ = 0;
        snprintf(s, 256, "-l%s", p);
        p = q;
        add(libs, s);
      }
    else if (!strncasecmp(buf, "//FLAGS", 7))
      for(p = buf + 7; *p;)
      {
        for(; isspace(*p); p++);
        for(q = p; !isspace(*q); q++);
        *q++ = 0;
        snprintf(s, 256, "%s", p);
        p = q;
        add(flags, s);
      }
    else if (!strncasecmp(buf, "//USES", 6))
      for (p = buf + 6; *p;)
      {
        for(; isspace(*p); p++);
        for(q = p; !isspace(*q); q++);
        *q++ = 0;
        result |= build(p, t);
        p = q;
      }
    else if (!strncasecmp(buf, "#include", 8))
    {
      for (p = buf + 8; isspace(*p); p++);
      if (*p++ == '"')
      {
        strtok(p, "\"");
        if (filedate(p) > t)
          result = 1;
        result |= dep(p, t);
      }
    }
  }
  fclose(f);
  return result;
}


/******************************************************
 *|F| build(char *fn, time_t t)
 *|F| Build .o from .c
 *|F|
 ******************************************************/
int build(char *fn, time_t t)
{
  /* fn = basename. fno = basename.o. fnc = basename.c */
  char fno[256];
  char fnc[256];
  char cmd[256];
  int result = 0;
  time_t tc, to;

  snprintf(fnc, 256, "%s.c", fn);
  snprintf(fno, 256, "%s.o", fn);
  add(objs, fno);
  tc = filedate(fnc);
  to = filedate(fno);
  result = dep(fnc, to) || (tc > to);
  if (result)
  {
    snprintf (cmd, 256, GCC_COMPILE, flags, fnc);
    puts(cmd);
    if (system(cmd))
      exit(1);
  }
  return result || (to > t);
}


/******************************************************
 *|F| void make(char *fn)
 *|F| Make an executable
 *|F|
 ******************************************************/
void make(char *fn)
{
  char cmd[256];
  *objs = *libs = *flags = 0;
  if (build(fn, filedate(fn)))
  {
    snprintf(cmd, 256, "gcc %s %s %s -o %s", flags, objs, libs, fn);
    puts(cmd);
    if (system(cmd))
      exit(1);
  }
  else
  {
    printf("'%s' is up to date.\n", fn);
  }
}



/******************************************************
 *|F| int main(int argc, char **argv)
 *|F|
 ******************************************************/
int main(int argc, char **argv)
{
  argc--, argv++;
  if (!argc)
  {
    printf("Usage:\n");
    printf("build <program>\n");
    return 1;
  }
  else
  {
    while(argc)
    {
      make(*argv);
      argc--;
      argv++;
    }
  }
  return 0;
}
