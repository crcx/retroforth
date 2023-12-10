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

V read_block(NgaState *vm) {
  CELL buffer = stack_pop(vm);
  CELL block = stack_pop(vm);
  int32_t m[1024];
  int fp = open(vm->BlockFile, O_RDONLY, 0666);
  lseek(fp, 4096 * block, SEEK_SET);
  read(fp, m, 4096);
  for (int i = 0; i < 1024; i++) {
    vm->memory[buffer + i] = (CELL)m[i];
  }
  close(fp);
}

V write_block(NgaState *vm) {
  CELL buffer = stack_pop(vm);
  CELL block = stack_pop(vm);
  int32_t m[1024];
  int fp = open(vm->BlockFile, O_WRONLY, 0666);
  lseek(fp, 4096 * block, SEEK_SET);
  for (int i = 0; i < 1024; i++) {
    m[i] = (int32_t)vm->memory[buffer + i];
  }
  write(fp, m, 4096);
  close(fp);
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
