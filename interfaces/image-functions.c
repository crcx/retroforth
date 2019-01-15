/* RETRO ------------------------------------------------------
  A personal, minimalistic forth
  Copyright (c) 2016 - 2019 Charles Childers

  This is a collection of functions for interacting with an
  image file.

  I'll include commentary throughout the source, so read on.
  ---------------------------------------------------------- */
  
/* ------------------------------------------------------------
  Begin by including the various C headers needed.
  ---------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>

#include "image-functions.h"

/* ------------------------------------------------------------
  A few variables. These are updated to point to the latest
  corresponding values in the image.
  ---------------------------------------------------------- */

CELL Compiler;
CELL Dictionary;
CELL NotFound;
CELL interpret;


/* ------------------------------------------------------------
  Now to the fun stuff: interfacing with the virtual machine.
  There are a things I like to have here:

  - push a value to the stack
  - pop a value off the stack
  - extract a string from the image
  - inject a string into the image.
  - lookup dictionary headers and access dictionary fields
  ---------------------------------------------------------- */


/*---------------------------------------------------------------------
  Stack push/pop is easy. I could avoid these, but it aids in keeping
  the code readable, so it's worth the slight overhead.
  ---------------------------------------------------------------------*/

CELL stack_pop() {
  sp--;
  if (sp < 0) {
    printf("Data stack underflow.\n");
    exit(1);
  }
  return data[sp + 1];
}

void stack_push(CELL value) {
  sp++;
  if (sp >= STACK_DEPTH) {
    printf("Data stack overflow.\n");
    exit(1);
  }
  data[sp] = value;
}


/*---------------------------------------------------------------------
  Strings are next. RETRO uses C-style NULL terminated strings. So I
  can easily inject or extract a string. Injection iterates over the
  string, copying it into the image. This also takes care to ensure
  that the NULL terminator is added.
  ---------------------------------------------------------------------*/

CELL string_inject(char *str, CELL buffer) {
  CELL m, i;
  if (!str) {
    memory[buffer] = 0;
    return 0;
  }
  m = strlen(str);
  i = 0;
  while (m > 0) {
    memory[buffer + i] = (CELL)str[i];
    memory[buffer + i + 1] = 0;
    m--; i++;
  }
  return buffer;
}


/*---------------------------------------------------------------------
  Extracting a string is similar, but I have to iterate over the VM
  memory instead of a C string and copy the charaters into a buffer.
  This uses a static buffer (`string_data`) as I prefer to avoid using
  `malloc()`.
  ---------------------------------------------------------------------*/

char string_data[8192];
char *string_extract(CELL at) {
  CELL starting = at;
  CELL i = 0;
  while(memory[starting] && i < 8192)
    string_data[i++] = (char)memory[starting++];
  string_data[i] = 0;
  return (char *)string_data;
}


/*---------------------------------------------------------------------
  Continuing along, I now define functions to access the dictionary.

  RETRO's dictionary is a linked list. Each entry is setup like:

  0000  Link to previous entry (NULL if this is the root entry)
  0001  Pointer to definition start
  0002  Pointer to class handler
  0003  Start of a NULL terminated string with the word name

  First, functions to access each field. The offsets were defineed at
  the start of the file.
  ---------------------------------------------------------------------*/

CELL d_link(CELL dt) {
  return dt + D_OFFSET_LINK;
}

CELL d_xt(CELL dt) {
  return dt + D_OFFSET_XT;
}

CELL d_class(CELL dt) {
  return dt + D_OFFSET_CLASS;
}

CELL d_name(CELL dt) {
  return dt + D_OFFSET_NAME;
}


/*---------------------------------------------------------------------
  Next, a more complext word. This will walk through the entries to
  find one with a name that matches the specified name. This is *slow*,
  but works ok unless you have a really large dictionary. (I've not
  run into issues with this in practice).
  ---------------------------------------------------------------------*/

CELL d_lookup(CELL Dictionary, char *name) {
  CELL dt = 0;
  CELL i = Dictionary;
  char *dname;
  while (memory[i] != 0 && i != 0) {
    dname = string_extract(d_name(i));
    if (strcmp(dname, name) == 0) {
      dt = i;
      i = 0;
    } else {
      i = memory[i];
    }
  }
  return dt;
}


/*---------------------------------------------------------------------
  My last dictionary related word returns the `xt` pointer for a word.
  This is used to help keep various important bits up to date.
  ---------------------------------------------------------------------*/

CELL d_xt_for(char *Name, CELL Dictionary) {
  return memory[d_xt(d_lookup(Dictionary, Name))];
}


/*---------------------------------------------------------------------
  This interface tracks a few words and variables in the image. These
  are:

  Dictionary - the latest dictionary header
  NotFound   - called when a word is not found
  interpret  - the heart of the interpreter/compiler

  I have to call this periodically, as the Dictionary will change as
  new words are defined, and the user might write a new error handler
  or interpreter.
  ---------------------------------------------------------------------*/

void update_rx() {
  Dictionary = memory[2];
  interpret = d_xt_for("interpret", Dictionary);
  NotFound = d_xt_for("err:notfound", Dictionary);
  Compiler = d_xt_for("err:notfound", Compiler);
}


/*---------------------------------------------------------------------
  With these out of the way, I implement `execute`, which takes an
  address and runs the code at it. This has a couple of interesting
  bits.

  Nga uses packed instruction bundles, with up to four instructions per
  bundle. Since RETRO requires an additional instruction to handle
  displaying a character, I define the handler for that here.

  This will also exit if the address stack depth is zero (meaning that
  the word being run, and it's dependencies) are finished.
  ---------------------------------------------------------------------*/

void execute(CELL cell) {
  CELL a, b, token;
  CELL opcode;
  rp = 1;
  ip = cell;
  token = TIB;
  while (ip < IMAGE_SIZE) {
    if (ip == NotFound) {
      printf("\nERROR: Word Not Found: ");
      printf("`%s`\n\n", string_extract(token));
    }
    if (ip == interpret) {
      token = TOS;
    }
    opcode = memory[ip];
    if (ngaValidatePackedOpcodes(opcode) != 0) {
      ngaProcessPackedOpcodes(opcode);
    } else {
      printf("Invalid instruction!\n");
      printf("At %d, opcode %d\n", ip, opcode);
      exit(1);
    }
    ip++;
    if (rp == 0)
      ip = IMAGE_SIZE;
  }
}



/*---------------------------------------------------------------------
  RETRO's `interpret` word expects a token on the stack. This next
  function copies a token to the `TIB` (text input buffer) and then
  calls `interpret` to process it.
  ---------------------------------------------------------------------*/

void evaluate(char *s) {
  if (strlen(s) == 0)
    return;
  update_rx();
  string_inject(s, TIB);
  stack_push(TIB);
  execute(interpret);
}
