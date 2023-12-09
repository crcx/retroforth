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

void io_blocks(NgaState *vm) {
  CELL op, buffer, block;
  int32_t m[1024];
  op = stack_pop(vm);

  if (op == 0) {
    buffer = stack_pop(vm);
    block = stack_pop(vm);
    int fp = open(vm->BlockFile, O_RDONLY, 0666);
    lseek(fp, 4096 * block, SEEK_SET);
    read(fp, m, 4096);
    for (int i = 0; i < 1024; i++) {
      vm->memory[buffer + i] = (CELL)m[i];
    }
    close(fp);
  }

  if (op == 1) {
    buffer = stack_pop(vm);
    block = stack_pop(vm);
    int fp = open(vm->BlockFile, O_WRONLY, 0666);
    lseek(fp, 4096 * block, SEEK_SET);
    for (int i = 0; i < 1024; i++) {
      m[i] = (int32_t)vm->memory[buffer + i];
    }
    write(fp, m, 4096);
    close(fp);
  }

  if (op == 2) {
    buffer = stack_pop(vm);
    strlcpy(vm->BlockFile, string_extract(vm, buffer), 1024);
  }
}

void query_blocks(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_BLOCKS);
}
