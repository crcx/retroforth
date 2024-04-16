/**************************************************************
                _              __            _   _
       _ __ ___| |_ _ __ ___  / _| ___  _ __| |_| |__
      | '__/ _ \ __| '__/ _ \| |_ / _ \| '__| __| '_ \
      | | |  __/ |_| | | (_) |  _| (_) | |  | |_| | | |
      |_|  \___|\__|_|  \___/|_|  \___/|_|   \__|_| |_|
                                                for nga

      (c) Charles Childers, Luke Parrish, Marc Simpsonn,
          Jay Skeer, Kenneth Keating

**************************************************************/


V utf32_to_utf8(uint32_t, unsigned char*, int*);
int fread_character(FILE *);

/*---------------------------------------------------------------------
  I keep an array of file handles. RETRO will use the index number as
  its representation of the file.
  ---------------------------------------------------------------------*/

/*---------------------------------------------------------------------
  `files_get_handle()` returns a file handle, or 0 if there are no
  available handle slots in the array.
  ---------------------------------------------------------------------*/

CELL files_get_handle(NgaState *vm) {
  for(CELL i = 1; i < MAX_OPEN_FILES; i++) {
    if (vm->OpenFileHandles[i] == 0) { return i; }
  }
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

V file_open(NgaState *vm) {
  CELL slot = files_get_handle(vm);
  CELL mode = stack_pop(vm);
  CELL name = stack_pop(vm);
  char *modes[] = {"rb", "w", "a", "rb+"};
  char *request = string_extract(vm, name);
  if (slot > 0) {
    vm->OpenFileHandles[slot] = fopen(request, modes[mode]);
  }
  if (vm->OpenFileHandles[slot] == NULL) {
    vm->OpenFileHandles[slot] = 0;
    slot = 0;
  }
  stack_push(vm, slot);
//  FILE *file = (slot > 0) ? fopen(request, modes[mode]) : NULL;
//  vm->OpenFileHandles[slot] = (file != NULL) ? file : 0;
//  stack_push(vm, slot);
}

/*---------------------------------------------------------------------
  `file_read()` reads a byte from a file. This takes a file pointer
  from the stack and pushes the character that was read to the stack.
  ---------------------------------------------------------------------*/

V file_read(NgaState *vm) {
  CELL slot = stack_pop(vm);
  FILE *file = vm->OpenFileHandles[slot];
  if (slot <= 0 || slot > MAX_OPEN_FILES || file == 0) {
    printf("\nERROR (nga/file_read): Invalid file handle\n");
    exit(1);
  }
  stack_push(vm, feof(file) ? 0 : fgetc(file));
}


/*--------------------------------------------------------------
  `file_write()` writes a byte to a file. This takes a file
  pointer (TOS) and a byte (NOS) from the stack. It does not
  return any values on the stack.
  ------------------------------------------------------------*/

V file_write(NgaState *vm) {
  CELL slot = stack_pop(vm);
  FILE *file = vm->OpenFileHandles[slot];
  if (slot <= 0 || slot > MAX_OPEN_FILES || file == 0) {
    printf("\nERROR (nga/file_write): Invalid file handle\n");
    exit(1);
  }
  fputc(stack_pop(vm), file);
}

/*--------------------------------------------------------------
  `file_close()` closes a file. This takes a file handle from
  the stack and does not return anything on the stack.
  ------------------------------------------------------------*/

V file_close(NgaState *vm) {
  CELL slot = stack_pop(vm);
  FILE *file = vm->OpenFileHandles[slot];
  if (slot <= 0 || slot > MAX_OPEN_FILES || file == 0) {
    printf("\nERROR (nga/file_close): Invalid file handle\n");
    exit(1);
  }
  fclose(file);
  vm->OpenFileHandles[slot] = 0;
}


/*--------------------------------------------------------------
  `file_get_position()` provides the current index into a file.
  This takes the file handle from the stack and returns the
  offset.
  ------------------------------------------------------------*/

V file_get_position(NgaState *vm) {
  CELL slot = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || vm->OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_get_position): Invalid file handle\n");
    exit(1);
  }
  stack_push(vm, (CELL) ftell(vm->OpenFileHandles[slot]));
}


/*--------------------------------------------------------------
  `file_set_position()` changes the current index into a file to
  the specified one. This takes a file handle (TOS) and new
  offset (NOS) from the stack.
  ------------------------------------------------------------*/

V file_set_position(NgaState *vm) {
  CELL slot, pos;
  slot = stack_pop(vm);
  pos  = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || vm->OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_set_position): Invalid file handle\n");
    exit(1);
  }
  fseek(vm->OpenFileHandles[slot], pos, SEEK_SET);
}


/*--------------------------------------------------------------
  `file_get_size()` returns the size of a file, or 0 if empty.
  If the file is a directory, it returns -1. It takes a file
  handle from the stack.
  ------------------------------------------------------------*/

V file_get_size(NgaState *vm) {
  CELL slot, current, r, size;
  struct stat buffer;
  slot = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || vm->OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_get_size): Invalid file handle\n");
    exit(1);
  }
  fstat(fileno(vm->OpenFileHandles[slot]), &buffer);
  if (!S_ISDIR(buffer.st_mode)) {
    current = ftell(vm->OpenFileHandles[slot]);
    r = fseek(vm->OpenFileHandles[slot], 0, SEEK_END);
    size = ftell(vm->OpenFileHandles[slot]);
    fseek(vm->OpenFileHandles[slot], current, SEEK_SET);
  } else {
    r = -1;
    size = 0;
  }
  stack_push(vm, (r == 0) ? size : 0);
}


/*--------------------------------------------------------------
  `file_delete()` removes a file. This takes a file name (as a
  string) from the stack.
  ------------------------------------------------------------*/

V file_delete(NgaState *vm) {
  char *request;
  CELL name = stack_pop(vm);
  request = string_extract(vm, name);
  unlink(request);
}


/*--------------------------------------------------------------
  `file_flush()` flushes any pending writes to disk. This takes
  a file handle from the stack.
  ------------------------------------------------------------*/

V file_flush(NgaState *vm) {
  CELL slot;
  slot = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || vm->OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_flush): Invalid file handle\n");
    exit(1);
  }
  fflush(vm->OpenFileHandles[slot]);
}

char file_bytes[32769];

V file_read_bytes(NgaState *vm) {
  CELL slot = stack_pop(vm);
  CELL size = stack_pop(vm);
  CELL dest = stack_pop(vm);
  CELL z = fread((char *)file_bytes, 1, size, vm->OpenFileHandles[slot]);
  for (CELL i = 0; i < size; i++) {
    CELL x = file_bytes[i];
    vm->memory[dest + i] = x;
  }
  stack_push(vm, z);
}

V file_write_bytes(NgaState *vm) {
  CELL slot = stack_pop(vm);
  CELL size = stack_pop(vm);
  CELL src  = stack_pop(vm);
  for (CELL i = 0; i < size; i++) {
    char x = vm->memory[src + i];
    file_bytes[i] = x;
  }
  CELL z = fwrite(&file_bytes, 1, size, vm->OpenFileHandles[slot]);
  stack_push(vm, z);
}

V file_read_character(NgaState *vm) {
  CELL c;
  CELL slot = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || vm->OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_read): Invalid file handle\n");
    exit(1);
  }
  c = fread_character(vm->OpenFileHandles[slot]);
  stack_push(vm, feof(vm->OpenFileHandles[slot]) ? 0 : c);
}

V file_write_character(NgaState *vm) {
  unsigned char utf8_bytes[4];
  int num_bytes;
  CELL slot, c, r;
  utf32_to_utf8(stack_pop(vm), utf8_bytes, &num_bytes);
  slot = stack_pop(vm);
  if (slot <= 0 || slot > MAX_OPEN_FILES || vm->OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_write): Invalid file handle\n");
    exit(1);
  }
  r = fwrite(&utf8_bytes, num_bytes, 1, vm->OpenFileHandles[slot]);
}

V file_read_line(NgaState *vm) {
  CELL slot = stack_pop(vm);
  CELL targ = stack_pop(vm);
  CELL c;
  if (slot <= 0 || slot > MAX_OPEN_FILES || vm->OpenFileHandles[slot] == 0) {
    printf("\nERROR (nga/file_read): Invalid file handle\n");
    exit(1);
  }
  c = fread_character(vm->OpenFileHandles[slot]);
  vm->memory[targ] = c;
  targ++;
  while (c != 10 && c != 13 && c != 0) {
    c = fread_character(vm->OpenFileHandles[slot]);
    vm->memory[targ] = c;
    targ++;
  }
  vm->memory[targ - 1] = 0;
}

V file_write_line(NgaState *vm) {
}

Handler FileActions[] = {
  file_open,          file_close,
  file_read,          file_write,
  file_get_position,  file_set_position,
  file_get_size,      file_delete,
  file_flush,         file_read_bytes,
  file_write_bytes,
  file_read_character,file_write_character,
  file_read_line,     file_write_line,
};

V query_filesystem(NgaState *vm) {
  stack_push(vm, 3);
  stack_push(vm, DEVICE_FILES);
}

V io_filesystem(NgaState *vm) {
  FileActions[stack_pop(vm)](vm);
}
