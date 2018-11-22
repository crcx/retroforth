/* RETRO --------------------------------------------------------------
  A personal, minimalistic forth
  Copyright (c) 2016 - 2018 Charles Childers

  This implements a file I/O device for use with RETRO.
  ---------------------------------------------------------------------*/

#define MAX_OPEN_FILES 128

/*---------------------------------------------------------------------
  Begin by including the various C headers needed.
  ---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

typedef void (*Handler)(void);

/*---------------------------------------------------------------------
  First, a few things that relate to the Nga virtual machine that RETRO
  runs on.
  ---------------------------------------------------------------------*/

#define CELL         int32_t      /* Cell size (32 bit, signed integer */
#define IMAGE_SIZE   524288 * 48  /* Amount of RAM. 12MiB by default.  */
#define ADDRESSES    2048         /* Depth of address stack            */
#define STACK_DEPTH  512          /* Depth of data stack               */

extern CELL sp, rp, ip;             /* Stack & instruction pointers    */
extern CELL data[STACK_DEPTH];      /* The data stack                  */
extern CELL address[ADDRESSES];     /* The address stack               */
extern CELL memory[IMAGE_SIZE + 1]; /* The memory for the image        */

#define TOS  data[sp]             /* Shortcut for top item on stack    */
#define NOS  data[sp-1]           /* Shortcut for second item on stack */
#define TORS address[rp]          /* Shortcut for top item on address stack */

/*---------------------------------------------------------------------
  Function prototypes.
  ---------------------------------------------------------------------*/

CELL stack_pop();
void stack_push(CELL value);
char *string_extract(CELL at);

/*---------------------------------------------------------------------
  I keep an array of file handles. RETRO will use the index number as
  its representation of the file.
  ---------------------------------------------------------------------*/

FILE *ioFileHandles[MAX_OPEN_FILES];

/*---------------------------------------------------------------------
  `ioGetFileHandle()` returns a file handle, or 0 if there are no
  available handle slots in the array.
  ---------------------------------------------------------------------*/

CELL ioGetFileHandle() {
  CELL i;
  for(i = 1; i < MAX_OPEN_FILES; i++)
    if (ioFileHandles[i] == 0)
      return i;
  return 0;
}


/*---------------------------------------------------------------------
  `ioOpenFile()` opens a file. This pulls from the RETRO data stack:

  - mode     (number, TOS)
  - filename (string, NOS)

  Modes are:

  | Mode | Corresponds To | Description          |
  | ---- | -------------- | -------------------- |
  |  0   | rb             | Open for reading     |
  |  1   | w              | Open for writing     |
  |  2   | a              | Open for append      |
  |  3   | rb+            | Open for read/update |

  The file name should be a NULL terminated string. This will attempt
  to open the requested file and will return a handle (index number
  into the `ioFileHandles` array).
  ---------------------------------------------------------------------*/

void ioOpenFile() {
  CELL slot, mode, name;
  char *request;
  slot = ioGetFileHandle();
  mode = data[sp]; sp--;
  name = data[sp]; sp--;
  request = string_extract(name);
  if (slot > 0) {
    if (mode == 0)  ioFileHandles[slot] = fopen(request, "rb");
    if (mode == 1)  ioFileHandles[slot] = fopen(request, "w");
    if (mode == 2)  ioFileHandles[slot] = fopen(request, "a");
    if (mode == 3)  ioFileHandles[slot] = fopen(request, "rb+");
  }
  if (ioFileHandles[slot] == NULL) {
    ioFileHandles[slot] = 0;
    slot = 0;
  }
  stack_push(slot);
}


/*---------------------------------------------------------------------
  `ioReadFile()` reads a byte from a file. This takes a file pointer
  from the stack and pushes the character that was read to the stack.
  ---------------------------------------------------------------------*/

void ioReadFile() {
  CELL slot = stack_pop();
  CELL c = fgetc(ioFileHandles[slot]);
  stack_push(feof(ioFileHandles[slot]) ? 0 : c);
}


/*---------------------------------------------------------------------
  `ioWriteFile()` writes a byte to a file. This takes a file pointer
  (TOS) and a byte (NOS) from the stack. It does not return any values
  on the stack.
  ---------------------------------------------------------------------*/

void ioWriteFile() {
  CELL slot, c, r;
  slot = stack_pop();
  c = stack_pop();
  r = fputc(c, ioFileHandles[slot]);
  c = (r == EOF) ? 0 : 1;
}


/*---------------------------------------------------------------------
  `ioCloseFile()` closes a file. This takes a file handle from the
  stack and does not return anything on the stack.
  ---------------------------------------------------------------------*/

void ioCloseFile() {
  fclose(ioFileHandles[data[sp]]);
  ioFileHandles[data[sp]] = 0;
  sp--;
}


/*---------------------------------------------------------------------
  `ioGetFilePosition()` provides the current index into a file. This
  takes the file handle from the stack and returns the offset.
  ---------------------------------------------------------------------*/

void ioGetFilePosition() {
  CELL slot = stack_pop();
  stack_push((CELL) ftell(ioFileHandles[slot]));
}


/*---------------------------------------------------------------------
  `ioSetFilePosition()` changes the current index into a file to the
  specified one. This takes a file handle (TOS) and new offset (NOS)
  from the stack.
  ---------------------------------------------------------------------*/

void ioSetFilePosition() {
  CELL slot, pos, r;
  slot = stack_pop();
  pos  = stack_pop();
  r = fseek(ioFileHandles[slot], pos, SEEK_SET);
}


/*---------------------------------------------------------------------
  `ioGetFileSize()` returns the size of a file, or 0 if empty. If the
  file is a directory, it returns -1. It takes a file handle from the
  stack.
  ---------------------------------------------------------------------*/

void ioGetFileSize() {
  CELL slot, current, r, size;
  struct stat buffer;
  int    status;
  slot = stack_pop();
  status = fstat(fileno(ioFileHandles[slot]), &buffer);
  if (!S_ISDIR(buffer.st_mode)) {
    current = ftell(ioFileHandles[slot]);
    r = fseek(ioFileHandles[slot], 0, SEEK_END);
    size = ftell(ioFileHandles[slot]);
    fseek(ioFileHandles[slot], current, SEEK_SET);
  } else {
    r = -1;
  }
  stack_push((r == 0) ? size : 0);
}


/*---------------------------------------------------------------------
  `ioDeleteFile()` removes a file. This takes a file name (as a string)
  from the stack.
  ---------------------------------------------------------------------*/

void ioDeleteFile() {
  char *request;
  CELL name = data[sp]; sp--;
  CELL result;
  request = string_extract(name);
  result = (unlink(request) == 0) ? -1 : 0;
}


/*---------------------------------------------------------------------
  `ioFlushFile()` flushes any pending writes to disk. This takes a
  file handle from the stack.
  ---------------------------------------------------------------------*/

void ioFlushFile() {
  CELL slot;
  slot = stack_pop();
  fflush(ioFileHandles[slot]);
}


Handler FileActions[10] = {
  ioOpenFile,
  ioCloseFile,
  ioReadFile,
  ioWriteFile,
  ioGetFilePosition,
  ioSetFilePosition,
  ioGetFileSize,
  ioDeleteFile,
  ioFlushFile
};

void io_filesystem_query() {
  stack_push(0);
  stack_push(4);
}

void io_filesystem_handler() {
  FileActions[stack_pop()]();
}
