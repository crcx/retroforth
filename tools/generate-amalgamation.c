/* RETRO amalgamation generator --------------------------------
   Write a standalone C version of the Nga runtime to standard
   output.
   ---------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH 1024
#define MAX_LINE 8192
#define MAX_INCLUDE_DEPTH 64
#define MAX_HEADERS 64

char included_headers[MAX_HEADERS][MAX_PATH];
int included_header_count;

void fail(const char *message, const char *path) {
  if (path == NULL)
    fprintf(stderr, "%s\n", message);
  else
    fprintf(stderr, "%s: %s\n", message, path);
  exit(1);
}

int include_path(const char *line, char *path, size_t size) {
  const char *start;
  const char *end;
  size_t length;

  if (strncmp(line, "#include \"", 10) != 0)
    return 0;

  start = line + 10;
  end = strchr(start, '"');
  if (end == NULL)
    return 0;

  length = (size_t)(end - start);
  if (length == 0 || length >= size)
    fail("Invalid include", line);

  memcpy(path, start, length);
  path[length] = '\0';
  return 1;
}

void directory_name(const char *path, char *directory, size_t size) {
  const char *slash;
  size_t length;

  slash = strrchr(path, '/');
  if (slash == NULL) {
    directory[0] = '\0';
    return;
  }

  length = (size_t)(slash - path + 1);
  if (length >= size)
    fail("Path is too long", path);
  memcpy(directory, path, length);
  directory[length] = '\0';
}

int is_header(const char *path) {
  size_t length;

  length = strlen(path);
  return length >= 2 && strcmp(path + length - 2, ".h") == 0;
}

int header_was_included(const char *path) {
  int i;

  for (i = 0; i < included_header_count; i++)
    if (strcmp(included_headers[i], path) == 0)
      return 1;

  if (included_header_count >= MAX_HEADERS)
    fail("Too many headers", path);
  strcpy(included_headers[included_header_count++], path);
  return 0;
}

void emit_file(const char *path, int depth) {
  FILE *fp;
  char line[MAX_LINE];
  char include[MAX_PATH];
  char directory[MAX_PATH];
  char include_file_name[MAX_PATH];
  size_t length;

  if (depth >= MAX_INCLUDE_DEPTH)
    fail("Include nesting is too deep", path);

  /* Header guards make repeated header inclusion harmless to the C
     preprocessor. Avoid expanding them repeatedly in the generated file. */
  if (is_header(path) && header_was_included(path))
    return;

  fp = fopen(path, "r");
  if (fp == NULL)
    fail("Unable to open", path);

  directory_name(path, directory, sizeof(directory));
  while (fgets(line, sizeof(line), fp) != NULL) {
    if (include_path(line, include, sizeof(include))) {
      if (strlen(directory) + strlen(include) >= sizeof(include_file_name))
        fail("Path is too long", include);
      strcpy(include_file_name, directory);
      strcat(include_file_name, include);
      emit_file(include_file_name, depth + 1);
    }
    else {
      fputs(line, stdout);
      length = strlen(line);
      if (length == 0 || line[length - 1] != '\n')
        fputc('\n', stdout);
    }
  }

  if (ferror(fp))
    fail("Unable to read", path);
  fclose(fp);
}

void emit_prelude(void) {
  puts("/* Build with: cc -O2 retro-unix.c -lm -o retro */");
  puts("");
  puts("#define ENABLE_FLOATS");
  puts("#define ENABLE_FILES");
  puts("#define ENABLE_UNIX");
  puts("#define ENABLE_RNG");
  puts("#define ENABLE_CLOCK");
  puts("#define ENABLE_SCRIPTING");
  puts("#define NEEDS_STRL");
  puts("");
  puts("#define BIT64");
  puts("#ifdef _WIN32");
  puts("#undef ENABLE_UNIX");
  puts("#undef ENABLE_RNG");
  puts("#endif");
  puts("");
}

int main(void) {
  emit_prelude();
  emit_file("vm/nga-c/retro.c", 0);
  emit_file("vm/nga-c/image_data.c", 0);
  emit_file("vm/nga-c/retro_modules.c", 0);
  return 0;
}
