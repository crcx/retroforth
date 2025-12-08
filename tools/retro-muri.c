/* RETRO ------------------------------------------------------
  A personal, minimalistic forth
  Copyright (c) 2017 - 2020 Charles Childers
  Copyright (c)        2019 Luke Parrish

  This is the standard assembler for Nga. It's primarily used
  to build the RETRO kernel (retro.muri).
  ---------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

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
#define MAX_NAMES 4096
#define STRING_LEN 64

#ifndef IMAGE_SIZE
#define IMAGE_SIZE 128 KiB
#endif

char Labels[MAX_NAMES][STRING_LEN];
CELL Pointers[MAX_NAMES];
CELL np;
char source[1 KiB];
CELL target[IMAGE_SIZE];
CELL here;
CELL last_dictionary_entry;
int dictionary_entries;

typedef void (*Handler)(char *);
void unu(char *, int, Handler);
size_t bsd_strlcpy(char *dst, const char *src, size_t dsize);
void parse_dict_line(char *buffer, char *name, char *label, char *class_name);
CELL dict_entry_size(char *name);


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


/* *********************************************************************** */

/* retro-unu is used to extract code blocks from a source file.
   I embed a minimal version of this here. */

char code_start[33], code_end[33], test_start[33], test_end[33];

void read_line(FILE *file, char *line_buffer) {
  int ch = getc(file);
  int count = 0;
  while ((ch != '\n') && (ch != EOF)) {
    line_buffer[count] = ch;
    count++;
    ch = getc(file);
  }
  line_buffer[count] = '\0';
}

int fence_boundary(char *buffer, int tests_enabled) {
  int flag = 1;
  if (strcmp(buffer, code_start) == 0) { flag = -1; }
  if (strcmp(buffer, code_end) == 0)   { flag = -1; }
  if (tests_enabled == 0) { return flag; }
  if (strcmp(buffer, test_start) == 0) { flag = -1; }
  if (strcmp(buffer, test_end) == 0)   { flag = -1; }
  return flag;
}

void unu(char *fname, int tests_enabled, Handler handler) {
  int inBlock = 0;
  char buffer[4096];
  FILE *fp;
  fp = fopen(fname, "r");
  if (fp == NULL) {
    printf("Unable to load file\n");
    exit(2);
  }
  while (!feof(fp)) {
    read_line(fp, buffer);
    if (fence_boundary(buffer, tests_enabled) == -1) {
      if (inBlock == 0) {
        inBlock = 1;
      } else {
        inBlock = 0;
      }
    } else {
      if (inBlock == 1) {
        handler(buffer);
      }
    }
  }
  fclose(fp);
}


/* *********************************************************************** */

/* save() writes the assembled image to a file named `ngaImage`. */

void save() {
  FILE *fp;
  if ((fp = fopen("ngaImage", "wb")) == NULL) {
    printf("Unable to save the image!\n");
    exit(2);
  }
  fwrite(&target, sizeof(CELL), here, fp);
  fclose(fp);
}


/* lookup() finds a label name and returs its offset in the image */

CELL lookup(char *name) {
  CELL slice = -1;
  CELL n = np;
  while (n > 0) {
    n--;
    if (strcmp(Labels[n], name) == 0) {
      slice = Pointers[n];
    }
  }
  return slice;
}


/* add_label() stores a name and location for later use by lookup() */

void add_label(char *name, CELL slice) {
  if (lookup(name) == -1) {
    bsd_strlcpy(Labels[np], name, STRING_LEN);
    Pointers[np] = slice;
    np++;
  } else {
    printf("Fatal error: %s already defined\n", name);
    exit(0);
  }
}


/* opcode_for() returns the opcode value for a two letter instruction name. */

CELL opcode_for(char *s) {
  char* opcodeList = "..lidudrswpupojucaccreeqneltgtfestadsumudianorxoshzrhaieiqii";
  int16_t* s16 = (int16_t *)s;
  int16_t* op16 = (int16_t *)opcodeList;
  int i = 0;
  for (i = 0; i <= 30; i++) {
    if (s16[0] == op16[i]) {
      return i;
    }
  }
  return 0;
}


/* the first pass records the address for each label */

void pass1(char *buffer) {
  char name[256], label[256], class_name[256];
  switch (buffer[0]) {
    case 'i': here++;                             break;
    case 'r': here++;                             break;
    case '-': here++;                             break;
    case 'd': here++;                             break;
    case 's': here = here + strlen(buffer) - 1;   break;
    case 'o': here = atoi(buffer+2);              break;
    case '*': here += atoi(buffer+2);             break;
    case 'D': parse_dict_line(buffer, name, label, class_name);
              here += dict_entry_size(name);
                                                  break;
    case ':': add_label(buffer+2, here);          break;
  }
}


/* the second pass assembles the instruction bundles */

void pass2(char *buffer) {
  unsigned int opcode;
  char inst[3];
  inst[2] = '\0';
  char name[256], label[256], class_name[256];
  switch (buffer[0]) {
    case 'i': memcpy(inst, buffer + 8, 2);
              opcode = opcode_for(inst);
              opcode = opcode << 8;
              memcpy(inst, buffer + 6, 2);
              opcode += opcode_for(inst);
              opcode = opcode << 8;
              memcpy(inst, buffer + 4, 2);
              opcode += opcode_for(inst);
              opcode = opcode << 8;
              memcpy(inst, buffer + 2, 2);
              opcode += opcode_for(inst);
              target[here++] = opcode;
              break;
    case 'o': here = atoi(buffer+2);              break;
    case '*': here += atoi(buffer+2);             break;
    case 'D': parse_dict_line(buffer, name, label, class_name);
              here += dict_entry_size(name);
                                                  break;
    case '-': here++;                             break;
    case 'r': here++;                             break;
    case 'd': here++;                             break;
    case 's': here = here + strlen(buffer) - 1;   break;
    case ':':                                     break;
  }
}


/* the third pass assembles raw numeric values */

void pass3(char *buffer) {
  char name[256], label[256], class_name[256];
  switch (buffer[0]) {
    case 'i': here++;                             break;
    case 'r': here++;                             break;
    case '-': here++;                             break;
    case 'o': here = atoi(buffer+2);              break;
    case '*': here += atoi(buffer+2);             break;
    case 'd': target[here++] = atoi(buffer+2);    break;
    case 'D': parse_dict_line(buffer, name, label, class_name);
              here += dict_entry_size(name);
                                                  break;
    case 's': here = here + strlen(buffer) - 1;   break;
    case ':':                                     break;
  }
}


/* the fourth pass assembles strings */

void pass4(char *buffer) {
  unsigned int opcode;
  char name[256], label[256], class_name[256];
  switch (buffer[0]) {
    case 'i': here++;                             break;
    case 'r': here++;                             break;
    case '-': here++;                             break;
    case 'd': here++;                             break;
    case 'o': here = atoi(buffer+2);              break;
    case '*': here += atoi(buffer+2);             break;
    case 's': opcode = 2;
              while (opcode < strlen(buffer)) {
                target[here++] = buffer[opcode++];
              }
              target[here++] = 0;
                                                  break;
    case 'D': {
                CELL start = here;
                parse_dict_line(buffer, name, label, class_name);
                CELL size = dict_entry_size(name);
                CELL string_start = start + 9;
                size_t len = strlen(name);
                for (size_t i = 0; i < len; i++) {
                  target[string_start + i] = name[i];
                }
                target[string_start + len] = 0;
                here += size;
              }
                                                  break;
    case ':':                                     break;
  }
}


/* the fifth pass resolves references to labels */

void pass5(char *buffer) {
  char name[256], label[256], class_name[256];
  switch (buffer[0]) {
    case 'i': here++;                             break;
    case 'o': here = atoi(buffer+2);              break;
    case '*': here += atoi(buffer+2);             break;
    case 'r': target[here++] = lookup(buffer+2);
              if (lookup(buffer+2) == -1)
                printf("Lookup failed: '%s'\n", buffer+2);
                                                  break;
    case '-': target[here++] = lookup(buffer+2);
              if (lookup(buffer+2) == -1)
                printf("Lookup failed: '%s'\n", buffer+2);
                                                  break;
    case 'd': here++;                             break;
    case 's': here = here + strlen(buffer) - 1;   break;
    case 'D': {
                CELL start = here;
                parse_dict_line(buffer, name, label, class_name);
                CELL size = dict_entry_size(name);
                CELL link = (dictionary_entries == 0) ? 0 : last_dictionary_entry;
                CELL xt = lookup(label);
                CELL class_handler = lookup(class_name);
                if (xt == -1) {
                  printf("Lookup failed: '%s'\n", label);
                }
                if (class_handler == -1) {
                  printf("Lookup failed: '%s'\n", class_name);
                }
                target[start + 0] = link;
                target[start + 1] = xt;
                target[start + 2] = class_handler;
                target[start + 3] = 0; /* source */
                target[start + 4] = 0; /* hash */
                target[start + 5] = 0; /* stack */
                target[start + 6] = 0; /* astack */
                target[start + 7] = 0; /* fstack */
                target[start + 8] = 0; /* descr */
                last_dictionary_entry = start;
                dictionary_entries++;
                here += size;
              }
                                                  break;
    case ':':                                     break;
  }
}



int main(int argc, char **argv) {
  bsd_strlcpy(code_start, "~~~", 32);
  bsd_strlcpy(code_end,   "~~~", 32);
  bsd_strlcpy(test_start, "```", 32);
  bsd_strlcpy(test_end,   "```", 32);
  last_dictionary_entry = 0;
  dictionary_entries = 0;
  np = 0;
  if (argc > 1) {
    here = 0; unu(argv[1], 0, &pass1);
    here = 0; unu(argv[1], 0, &pass2);
    here = 0; unu(argv[1], 0, &pass3);
    here = 0; unu(argv[1], 0, &pass4);
    here = 0; unu(argv[1], 0, &pass5);
    save();
    printf("Wrote %lld cells to ngaImage\n", (long long)here);
  }
  else {
    printf("No file specified.\n");
  }
  return 0;
}


/* *********************************************************************** */

void parse_dict_line(char *buffer, char *name, char *label, char *class_name) {
  char *p = buffer + 2;
  int field = 0;
  char *dest = name;
  size_t max = 256;
  name[0] = label[0] = class_name[0] = '\0';
  while (*p == ' ' || *p == '\t') { p++; }
  while (*p != '\0') {
    if (*p == ' ' || *p == '\t') {
      while (*p == ' ' || *p == '\t') { p++; }
      field++;
      if (field == 1) { dest = label; max = 256; label[0] = '\0'; }
      else if (field == 2) { dest = class_name; max = 256; class_name[0] = '\0'; }
      else { break; }
      continue;
    }
    size_t len = strlen(dest);
    if (len + 1 < max) {
      dest[len] = *p;
      dest[len + 1] = '\0';
    }
    p++;
  }
}

CELL dict_entry_size(char *name) {
  return 9 + (CELL)strlen(name) + 1;
}


/* GNU libc does not include the strl* functions available on BSD systems.
   I include a copy of the OpenBSD implementation here as I'd rather use
   the safer version of the functions.                                     */

/*
 * Copyright (c) 1998, 2015 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/*
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
 */
size_t bsd_strlcpy(char *dst, const char *src, size_t dsize)
{
	const char *osrc = src;
	size_t nleft = dsize;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0';		/* NUL-terminate dst */
		while (*src++)
			;
	}

	return(src - osrc - 1);	/* count does not include NUL */
}
