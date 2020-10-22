
/*---------------------------------------------------------------------
  File Handling
  ---------------------------------------------------------------------*/


/*---------------------------------------------------------------------
  I keep an array of file handles. RETRO will use the index number as
  its representation of the file.
  ---------------------------------------------------------------------*/

FILE *OpenFileHandles[MAX_OPEN_FILES];

/*---------------------------------------------------------------------
  `files_get_handle()` returns a file handle, or 0 if there are no
  available handle slots in the array.
  ---------------------------------------------------------------------*/

CELL files_get_handle() {
  CELL i;
  for(i = 1; i < MAX_OPEN_FILES; i++)
    if (OpenFileHandles[i] == 0)
      return i;
  return 0;
}


/*---------------------------------------------------------------------
  `file_open()` opens a file. This pulls from the RETRO data stack:

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
  into the `OpenFileHandles` array).
  ---------------------------------------------------------------------*/

void file_open() {
  CELL slot, mode, name;
  char *request;
  slot = files_get_handle();
  mode = stack_pop();
  name = stack_pop();
  request = string_extract(name);
  if (slot > 0) {
    if (mode == 0)  OpenFileHandles[slot] = fopen(request, "rb");
    if (mode == 1)  OpenFileHandles[slot] = fopen(request, "w");
    if (mode == 2)  OpenFileHandles[slot] = fopen(request, "a");
    if (mode == 3)  OpenFileHandles[slot] = fopen(request, "rb+");
  }
  if (OpenFileHandles[slot] == NULL) {
    OpenFileHandles[slot] = 0;
    slot = 0;
  }
  stack_push(slot);
}


/*---------------------------------------------------------------------
  `file_read()` reads a byte from a file. This takes a file pointer
  from the stack and pushes the character that was read to the stack.
  ---------------------------------------------------------------------*/

void file_read() {
  CELL c;
  CELL slot = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_read): Invalid file handle\n");
    exit(1);
  }
#endif
  c = fgetc(OpenFileHandles[slot]);
  stack_push(feof(OpenFileHandles[slot]) ? 0 : c);
}


/*---------------------------------------------------------------------
  `file_write()` writes a byte to a file. This takes a file pointer
  (TOS) and a byte (NOS) from the stack. It does not return any values
  on the stack.
  ---------------------------------------------------------------------*/

void file_write() {
  CELL slot, c, r;
  slot = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_write): Invalid file handle\n");
    exit(1);
  }
#endif
  c = stack_pop();
  r = fputc(c, OpenFileHandles[slot]);
}


/*---------------------------------------------------------------------
  `file_close()` closes a file. This takes a file handle from the
  stack and does not return anything on the stack.
  ---------------------------------------------------------------------*/

void file_close() {
  CELL slot = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_close): Invalid file handle\n");
    exit(1);
  }
#endif
  fclose(OpenFileHandles[slot]);
  OpenFileHandles[slot] = 0;
}


/*---------------------------------------------------------------------
  `file_get_position()` provides the current index into a file. This
  takes the file handle from the stack and returns the offset.
  ---------------------------------------------------------------------*/

void file_get_position() {
  CELL slot = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_get_position): Invalid file handle\n");
    exit(1);
  }
#endif
  stack_push((CELL) ftell(OpenFileHandles[slot]));
}


/*---------------------------------------------------------------------
  `file_set_position()` changes the current index into a file to the
  specified one. This takes a file handle (TOS) and new offset (NOS)
  from the stack.
  ---------------------------------------------------------------------*/

void file_set_position() {
  CELL slot, pos;
  slot = stack_pop();
  pos  = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_set_position): Invalid file handle\n");
    exit(1);
  }
#endif
  fseek(OpenFileHandles[slot], pos, SEEK_SET);
}


/*---------------------------------------------------------------------
  `file_get_size()` returns the size of a file, or 0 if empty. If the
  file is a directory, it returns -1. It takes a file handle from the
  stack.
  ---------------------------------------------------------------------*/

void file_get_size() {
  CELL slot, current, r, size;
  struct stat buffer;
  slot = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_get_size): Invalid file handle\n");
    exit(1);
  }
#endif
  fstat(fileno(OpenFileHandles[slot]), &buffer);
  if (!S_ISDIR(buffer.st_mode)) {
    current = ftell(OpenFileHandles[slot]);
    r = fseek(OpenFileHandles[slot], 0, SEEK_END);
    size = ftell(OpenFileHandles[slot]);
    fseek(OpenFileHandles[slot], current, SEEK_SET);
  } else {
    r = -1;
    size = 0;
  }
  stack_push((r == 0) ? size : 0);
}


/*---------------------------------------------------------------------
  `file_delete()` removes a file. This takes a file name (as a string)
  from the stack.
  ---------------------------------------------------------------------*/

void file_delete() {
  char *request;
  CELL name = stack_pop();
  request = string_extract(name);
  unlink(request);
}


/*---------------------------------------------------------------------
  `file_flush()` flushes any pending writes to disk. This takes a
  file handle from the stack.
  ---------------------------------------------------------------------*/

void file_flush() {
  CELL slot;
  slot = stack_pop();
#ifndef NOCHECKS
  if (slot <= 0 || slot > MAX_OPEN_FILES || OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_flush): Invalid file handle\n");
    exit(1);
  }
#endif
  fflush(OpenFileHandles[slot]);
}


Handler FileActions[10] = {
  file_open,
  file_close,
  file_read,
  file_write,
  file_get_position,
  file_set_position,
  file_get_size,
  file_delete,
  file_flush
};

void io_filesystem_query() {
  stack_push(0);
  stack_push(4);
}

void io_filesystem_handler() {
  FileActions[stack_pop()]();
}
