/*---------------------------------------------------------------------
  RETRO is a personal, minimalistic forth with a pragmatic focus

  This implements Nga, the virtual machine at the heart of RETRO. It
  includes a number of I/O interfaces, extensive commentary, and has
  been refined by over a decade of use and development.

  Copyright (c) 2008 - 2021, Charles Childers

  Portions are based on Ngaro, which was additionally copyrighted by
  the following:

  Copyright (c) 2009 - 2010, Luke Parrish
  Copyright (c) 2010,        Marc Simpson
  Copyright (c) 2010,        Jay Skeer
  Copyright (c) 2011,        Kenneth Keating
  ---------------------------------------------------------------------*/


/*---------------------------------------------------------------------
  C Headers
  ---------------------------------------------------------------------*/

#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

/*---------------------------------------------------------------------
  Configuration
  ---------------------------------------------------------------------*/

#include "config.h"

/*---------------------------------------------------------------------
  Image, Stack, and VM variables
  ---------------------------------------------------------------------*/

CELL memory[IMAGE_SIZE + 1];      /* The memory for the image          */

#define TOS  cpu.data[cpu.sp]     /* Shortcut for top item on stack    */
#define NOS  cpu.data[cpu.sp-1]   /* Shortcut for second item on stack */
#define TORS cpu.address[cpu.rp]  /* Shortcut for top item on address stack */

struct NgaCore {
  CELL sp, rp, ip;                /* Stack & instruction pointers      */
  CELL data[STACK_DEPTH];         /* The data stack                    */
  CELL address[ADDRESSES];        /* The address stack                 */
} cpu;



#include "prototypes.h"

void loadEmbeddedImage(char *arg);


/*---------------------------------------------------------------------
  Populate The I/O Device Tables
  ---------------------------------------------------------------------*/

typedef void (*Handler)(void);

Handler IO_deviceHandlers[NUM_DEVICES + 1] = {
  io_output_handler,
  io_keyboard_handler,
  io_filesystem_handler,
  io_floatingpoint_handler,
  io_scripting_handler,
  io_unix_handler,
  io_clock_handler,
  io_image,
  io_random,
};

Handler IO_queryHandlers[NUM_DEVICES + 1] = {
  io_output_query,
  io_keyboard_query,
  io_filesystem_query,
  io_floatingpoint_query,
  io_scripting_query,
  io_unix_query,
  io_clock_query,
  io_image_query,
  io_random_query,
};


/*---------------------------------------------------------------------
  Variables Related To Image Introspection
  ---------------------------------------------------------------------*/

CELL Compiler;
CELL Dictionary;
CELL NotFound;
CELL interpret;


/*---------------------------------------------------------------------
  Embed The Image
  ---------------------------------------------------------------------*/

#include "retro-image.c"
#include "bsd-strl.c"


/*---------------------------------------------------------------------
  Global Variables
  ---------------------------------------------------------------------*/

char string_data[8192];
char **sys_argv;
int sys_argc;
int silence_input;


/*=====================================================================*/


/*---------------------------------------------------------------------
  Now on to I/O and extensions!

  RRE provides a lot of additional functionality over the base RETRO
  system. First up is support for files.

  The RRE file model is intended to be similar to that of the standard
  C libraries and wraps fopen(), fclose(), etc.
  ---------------------------------------------------------------------*/

void io_output_handler() {
  putc(stack_pop(), stdout);
  fflush(stdout);
}

void io_output_query() {
  stack_push(0);
  stack_push(0);
}


/*=====================================================================*/


void io_keyboard_handler() {
  stack_push(getc(stdin));
  if (TOS == 127) TOS = 8;
}

void io_keyboard_query() {
  stack_push(0);
  stack_push(1);
}


/*=====================================================================*/


/*---------------------------------------------------------------------
  Scripting Support
  ---------------------------------------------------------------------*/

void scripting_arg() {
  CELL a, b;
  a = stack_pop();
  b = stack_pop();
  stack_push(string_inject(sys_argv[a + 1], b));
}

void scripting_arg_count() {
  stack_push(sys_argc - 1);
}

void scripting_include() {
  include_file(string_extract(stack_pop()), 0);
}

void scripting_name() {
  stack_push(string_inject(sys_argv[0], stack_pop()));
}

Handler ScriptingActions[] = {
  scripting_arg_count,
  scripting_arg,
  scripting_include,
  scripting_name
};

void io_scripting_query() {
  stack_push(0);
  stack_push(9);
}

void io_scripting_handler() {
  ScriptingActions[stack_pop()]();
}


/*=====================================================================*/

#include "dev-image.c"
#include "dev-rng.c"
#include "dev-floatingpoint.c"
#include "dev-files.c"
#include "dev-unix.c"
#include "dev-clock.c"

/*=====================================================================*/


/*=====================================================================*/


/*---------------------------------------------------------------------
  With these out of the way, I implement `execute`, which takes an
  address and runs the code at it. This has a couple of interesting
  bits.

  This will also exit if the address stack depth is zero (meaning that
  the word being run, and it's dependencies) are finished.
  ---------------------------------------------------------------------*/

void rre_execute(CELL cell, int silent) {
  CELL a, b, token;
  CELL opcode;
  silence_input = silent;
  cpu.rp = 1;
  cpu.ip = cell;
  token = TIB;
  while (cpu.ip < IMAGE_SIZE) {
    if (cpu.ip == NotFound) {
      printf("\nERROR: Word Not Found: ");
      printf("`%s`\n\n", string_extract(token));
    }
    if (cpu.ip == interpret) {
      token = TOS;
    }
    opcode = memory[cpu.ip];
    if (validate_opcode_bundle(opcode) != 0) {
      process_opcode_bundle(opcode);
    } else {
      printf("Invalid instruction!\n");
      printf("At %d, opcode %d\n", cpu.ip, opcode);
      exit(1);
    }
    if (cpu.sp < 0 || cpu.sp > STACK_DEPTH) {
      printf("\nStack Limits Exceeded!\n");
      printf("At %d, opcode %d\n", cpu.ip, opcode);
      exit(1);
    }
    cpu.ip++;
    if (cpu.rp == 0)
      cpu.ip = IMAGE_SIZE;
  }
}

/*---------------------------------------------------------------------
  RETRO's `interpret` word expects a token on the stack. This next
  function copies a token to the `TIB` (text input buffer) and then
  calls `interpret` to process it.
  ---------------------------------------------------------------------*/

void rre_evaluate(char *s, int silent) {
  if (strlen(s) == 0)  return;
  update_rx();
  string_inject(s, TIB);
  stack_push(TIB);
  rre_execute(interpret, silent);
}


/*---------------------------------------------------------------------
  `read_token` reads a token from the specified file.  It will stop on
   a whitespace or newline. It also tries to handle backspaces, though
   the success of this depends on how your terminal is configured.
  ---------------------------------------------------------------------*/

int not_eol(int ch) {
  return (ch != 10) && (ch != 13) && (ch != 32) && (ch != EOF) && (ch != 0);
}

void read_token(FILE *file, char *token_buffer, int echo) {
  int ch = getc(file);
  int count = 0;
  if (echo != 0)
    putchar(ch);
  while (not_eol(ch))
  {
    if ((ch == 8 || ch == 127) && count > 0) {
      count--;
      if (echo != 0) {
        putchar(8);
        putchar(32);
        putchar(8);
      }
    } else {
      token_buffer[count++] = ch;
    }
    ch = getc(file);
    if (echo != 0)
      putchar(ch);
  }
  token_buffer[count] = '\0';
}


/*---------------------------------------------------------------------
  Display the Stack Contents
  ---------------------------------------------------------------------*/

void dump_stack() {
  CELL i;
  if (cpu.sp == 0)  return;
  printf("\nStack: ");
  for (i = 1; i <= cpu.sp; i++) {
    if (i == cpu.sp)
      printf("[ TOS: %d ]", cpu.data[i]);
    else
      printf("%d ", cpu.data[i]);
  }
  printf("\n");
}


/*---------------------------------------------------------------------
  RRE is primarily intended to be used in a batch or scripting model.
  The `include_file()` function will be used to read the code in the
  file, evaluating it as encountered.

  I enforce a literate model, with code in fenced blocks. E.g.,

    # This is a test

    Display "Hello, World!"

    ~~~
    'Hello,_World! puts nl
    ~~~

  RRE will ignore anything outside the `~~~` blocks. To identify if the
  current token is the start or end of a block, I provide a `fenced()`
  function.
  ---------------------------------------------------------------------*/

int fenced(char *s)
{
  int a = strcmp(s, "```");
  int b = strcmp(s, "~~~");
  if (a == 0) return 2;
  if (b == 0) return 1;
              return 0;
}


/*---------------------------------------------------------------------
  And now for the actual `include_file()` function.
  ---------------------------------------------------------------------*/

void include_file(char *fname, int run_tests) {
  int inBlock = 0;                 /* Tracks status of in/out of block */
  char source[64 * 1024];          /* Line buffer [about 64K]          */
  char fence[4];                   /* Used with `fenced()`             */

  FILE *fp;                        /* Open the file. If not found,     */
  fp = fopen(fname, "r");          /* exit.                            */
  if (fp == NULL)
    return;

  while (!feof(fp))                /* Loop through the file            */
  {
    read_token(fp, source, 0);
    strncpy(fence, source, 3);     /* Copy the first three characters  */
    fence[3] = '\0';               /* into `fence` to see if we are in */
    if (fenced(fence) > 0) {       /* a code block.                    */
      if (fenced(fence) == 2 && run_tests == 0) {
      } else {
        if (inBlock == 0)
          inBlock = 1;
        else
          inBlock = 0;
      }
    } else {
      if (inBlock == 1)            /* If we are, evaluate token        */
        rre_evaluate(source, -1);
    }
  }

  fclose(fp);
}


/*---------------------------------------------------------------------
  `help()` displays a summary of the command line arguments RRE allows.

  This is invoked using `rre -h`
  ---------------------------------------------------------------------*/

void help(char *exename) {
  printf("Scripting Usage: %s filename\n\n", exename);
  printf("Interactive Usage: %s [-h] [-i[,fs]] [-s] [-f filename] [-t]\n\n", exename);
  printf("Valid Arguments:\n\n");
  printf("  -h\n");
  printf("    Display this help text\n");
  printf("  -i\n");
  printf("    Launches in interactive mode (line buffered)\n");
  printf("  -i,fs\n");
  printf("    Launches in interactive mode (character buffered, full screen)\n");
  printf("  -s\n");
  printf("    Suppress the 'ok' prompt and keyboard echo in interactive mode\n");
  printf("  -f filename\n");
  printf("    Run the contents of the specified file\n");
  printf("  -u filename\n");
  printf("    Use the image in the specified file instead of the internal one\n");
  printf("  -t\n");
  printf("    Run tests (in ``` blocks) in any loaded files\n\n");
}


/*---------------------------------------------------------------------
  `initialize()` sets up Nga and loads the image (from the array in
  `image.c`) to memory.
  ---------------------------------------------------------------------*/

void initialize() {
  CELL i;
  prepare_vm();
  for (i = 0; i < ngaImageCells; i++)
    memory[i] = ngaImage[i];
  update_rx();
}


/*---------------------------------------------------------------------
  `arg_is()` exists to aid in readability. It compares the first actual
  command line argument to a string and returns a boolean flag.
  ---------------------------------------------------------------------*/

int arg_is(char *argv, char *t) {
  return strcmp(argv, t) == 0;
}


/*---------------------------------------------------------------------
  Main Entry Point
  ---------------------------------------------------------------------*/

enum flags {
  FLAG_HELP, FLAG_RUN_TESTS, FLAG_INCLUDE, FLAG_INTERACTIVE, FLAG_SILENT,
  FLAG_FULLSCREEN
};

int main(int argc, char **argv) {
  sys_argc = argc;                        /* Point the global argc and */
  sys_argv = argv;                        /* argv to the actual ones   */

  prepare_vm();
  loadEmbeddedImage(argv[0]);
  update_rx();
  rre_execute(0, 0);
  exit(0);
}


/*---------------------------------------------------------------------
  Interfacing With The Image
  ---------------------------------------------------------------------*/

/*---------------------------------------------------------------------
  Stack push/pop is easy. I could avoid these, but it aids in keeping
  the code readable, so it's worth the slight overhead.
  ---------------------------------------------------------------------*/

CELL stack_pop() {
  cpu.sp--;
  if (cpu.sp < 0) {
    printf("Data stack underflow.\n");
    exit(1);
  }
  return cpu.data[cpu.sp + 1];
}

void stack_push(CELL value) {
  cpu.sp++;
  if (cpu.sp >= STACK_DEPTH) {
    printf("Data stack overflow.\n");
    exit(1);
  }
  cpu.data[cpu.sp] = value;
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
  Compiler = d_xt_for("Compiler", Compiler);
}


/*=====================================================================*/

#include "nga.c"

/*=====================================================================*/


#pragma pack(push,1)
#pragma pack(pop)

#define EI_NIDENT       16

/* 32-bit ELF base types. */
typedef unsigned int Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Off;
typedef signed int Elf32_Sword;
typedef unsigned int Elf32_Word;

/* 64-bit ELF base types. */
typedef unsigned long long Elf64_Addr;
typedef unsigned short Elf64_Half;
typedef signed short Elf64_SHalf;
typedef unsigned long long Elf64_Off;
typedef signed int Elf64_Sword;
typedef unsigned int Elf64_Word;
typedef unsigned long long Elf64_Xword;
typedef signed long long Elf64_Sxword;

typedef struct elf32_hdr{
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half    e_type;
  Elf32_Half    e_machine;
  Elf32_Word    e_version;
  Elf32_Addr    e_entry;  /* Entry point */
  Elf32_Off e_phoff;
  Elf32_Off e_shoff;
  Elf32_Word    e_flags;
  Elf32_Half    e_ehsize;
  Elf32_Half    e_phentsize;
  Elf32_Half    e_phnum;
  Elf32_Half    e_shentsize;
  Elf32_Half    e_shnum;
  Elf32_Half    e_shstrndx;
} Elf32_Ehdr;

typedef struct elf32_shdr {
  Elf32_Word    sh_name;
  Elf32_Word    sh_type;
  Elf32_Word    sh_flags;
  Elf32_Addr    sh_addr;
  Elf32_Off sh_offset;
  Elf32_Word    sh_size;
  Elf32_Word    sh_link;
  Elf32_Word    sh_info;
  Elf32_Word    sh_addralign;
  Elf32_Word    sh_entsize;
} Elf32_Shdr;

typedef struct elf64_hdr {
  unsigned char e_ident[EI_NIDENT]; /* ELF "magic number" */
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;       /* Entry point virtual address */
  Elf64_Off e_phoff;        /* Program header table file offset */
  Elf64_Off e_shoff;        /* Section header table file offset */
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct elf64_shdr {
  Elf64_Word sh_name;       /* Section name, index in string tbl */
  Elf64_Word sh_type;       /* Type of section */
  Elf64_Xword sh_flags;     /* Miscellaneous section attributes */
  Elf64_Addr sh_addr;       /* Section virtual addr at execution */
  Elf64_Off sh_offset;      /* Section file offset */
  Elf64_Xword sh_size;      /* Size of section in bytes */
  Elf64_Word sh_link;       /* Index of another section */
  Elf64_Word sh_info;       /* Additional section information */
  Elf64_Xword sh_addralign; /* Section alignment */
  Elf64_Xword sh_entsize;   /* Entry size if section holds table */
} Elf64_Shdr;

void loadEmbeddedImage(char *arg) {
  FILE* ElfFile = NULL;
  char* SectNames = NULL;
  Elf64_Ehdr elfHdr;
  Elf64_Shdr sectHdr;
  uint32_t idx;

  if((ElfFile = fopen(arg, "r")) == NULL) {
    perror("[E] Error opening file:");
    exit(1);
  }

  fread(&elfHdr, 1, sizeof(Elf64_Ehdr), ElfFile);
  fseek(ElfFile, elfHdr.e_shoff + elfHdr.e_shstrndx * sizeof(sectHdr), SEEK_SET);
  fread(&sectHdr, 1, sizeof(sectHdr), ElfFile);

  SectNames = malloc(sectHdr.sh_size);
  fseek(ElfFile, sectHdr.sh_offset, SEEK_SET);
  fread(SectNames, 1, sectHdr.sh_size, ElfFile);

  int a;

  for (idx = 0; idx < elfHdr.e_shnum; idx++)
  {
    const char* name = "";

    fseek(ElfFile, elfHdr.e_shoff + idx * sizeof(sectHdr), SEEK_SET);
    fread(&sectHdr, 1, sizeof(sectHdr), ElfFile);
    name = SectNames + sectHdr.sh_name;
    if (strcmp(name, ".ngaImage") == 0) {
      fseek(ElfFile, sectHdr.sh_offset, SEEK_SET);
      for (int i = 0; i < (int)sectHdr.sh_size; i++) {
        fread(&a, 1, sizeof(int), ElfFile);
        memory[i] = a;
      }
    }
  }
  return;
}
