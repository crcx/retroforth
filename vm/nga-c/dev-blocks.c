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

#define BLOCK_CELLS 1024
#define BLOCK_BYTES (BLOCK_CELLS * (int)sizeof(int32_t))

int blocks_validate(CELL block, CELL buffer) {
  uintmax_t block_number;
  if (buffer < 0 || buffer > IMAGE_SIZE - BLOCK_CELLS) {
    fprintf(stderr, "ERROR (nga/blocks): Invalid memory range\n");
    return 0;
  }
  if (block < 0 || (CELL)(off_t)block != block) {
    fprintf(stderr, "ERROR (nga/blocks): Invalid block number\n");
    return 0;
  }
  block_number = (uintmax_t)block;
  if (block_number > (uintmax_t)INT64_MAX / BLOCK_BYTES) {
    fprintf(stderr, "ERROR (nga/blocks): Invalid block number\n");
    return 0;
  }
  return 1;
}

int blocks_seek(int fp, CELL block) {
  if (lseek(fp, (off_t)block * BLOCK_BYTES, SEEK_SET) == (off_t)-1) {
    perror("ERROR (nga/blocks): Unable to seek block file");
    return 0;
  }
  return 1;
}

int blocks_read_all(int fp, void *buffer, size_t size) {
  char *at = buffer;
  while (size > 0) {
    ssize_t count = read(fp, at, size);
    if (count > 0) {
      at += count;
      size -= count;
    } else if (count == 0) {
      fprintf(stderr, "ERROR (nga/blocks): Unexpected end of block file\n");
      return 0;
    } else if (errno != EINTR) {
      perror("ERROR (nga/blocks): Unable to read block file");
      return 0;
    }
  }
  return 1;
}

int blocks_write_all(int fp, const void *buffer, size_t size) {
  const char *at = buffer;
  while (size > 0) {
    ssize_t count = write(fp, at, size);
    if (count > 0) {
      at += count;
      size -= count;
    } else if (count == 0) {
      fprintf(stderr, "ERROR (nga/blocks): Unable to write block file\n");
      return 0;
    } else if (errno != EINTR) {
      perror("ERROR (nga/blocks): Unable to write block file");
      return 0;
    }
  }
  return 1;
}

V read_block(NgaState *vm) {
  CELL buffer = stack_pop(vm);
  CELL block = stack_pop(vm);
  int32_t m[BLOCK_CELLS];
  if (!blocks_validate(block, buffer)) return;

  int fp = open(vm->BlockFile, O_RDONLY);
  if (fp == -1) {
    perror("ERROR (nga/blocks): Unable to open block file");
    return;
  }
  if (blocks_seek(fp, block) && blocks_read_all(fp, m, BLOCK_BYTES)) {
    for (int i = 0; i < BLOCK_CELLS; i++) {
      vm->memory[buffer + i] = (CELL)m[i];
    }
  }
  if (close(fp) == -1) {
    perror("ERROR (nga/blocks): Unable to close block file");
  }
}

V write_block(NgaState *vm) {
  CELL buffer = stack_pop(vm);
  CELL block = stack_pop(vm);
  int32_t m[BLOCK_CELLS];
  if (!blocks_validate(block, buffer)) return;

  for (int i = 0; i < BLOCK_CELLS; i++) {
    m[i] = (int32_t)vm->memory[buffer + i];
  }
  int fp = open(vm->BlockFile, O_WRONLY);
  if (fp == -1) {
    perror("ERROR (nga/blocks): Unable to open block file");
    return;
  }
  if (blocks_seek(fp, block)) {
    blocks_write_all(fp, m, BLOCK_BYTES);
  }
  if (close(fp) == -1) {
    perror("ERROR (nga/blocks): Unable to close block file");
  }
}

V set_block_file(NgaState *vm) {
  CELL buffer = stack_pop(vm);
  strlcpy(vm->BlockFile, string_extract(vm, buffer), 1024);
}

V io_blocks(NgaState *vm) {
  switch (stack_pop(vm)) {
    case 0: read_block(vm); break;
    case 1: write_block(vm); break;
    case 2: set_block_file(vm); break;
  }
}

V query_blocks(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_BLOCKS);
}
