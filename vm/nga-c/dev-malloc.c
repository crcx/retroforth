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


#ifdef ENABLE_MALLOC
#ifdef BIT64
typedef union {
  V* val;
  struct {
    CELL msw;
    CELL lsw;
  };
} double_cell;

V double_add(NgaState *vm) {
  double_cell a;
  double_cell b;
  double_cell c;
  b.msw = stack_pop(vm);
  b.lsw = stack_pop(vm);
  a.msw = stack_pop(vm);
  a.lsw = stack_pop(vm);
}

V double_sub(NgaState *vm) {
}

V double_mul(NgaState *vm) {
}

V double_divmod(NgaState *vm) {
}

V malloc_allocate(NgaState *vm) {
  stack_push(vm, (CELL)malloc(stack_pop(vm)));
}

V malloc_free(NgaState *vm) {
  free((CELL*)stack_pop(vm));
}

V malloc_store(NgaState *vm) {
  CELL value = stack_pop(vm);
  double_cell addr;
  *(CELL *) stack_pop(vm) = value;
}

V malloc_fetch(NgaState *vm) {
  double_cell addr;
  CELL value = *(CELL *)stack_pop(vm);
  stack_push(vm, value);
}

V malloc_realloc(NgaState *vm) {
  CELL bytes = stack_pop(vm);
  CELL* addr1;
  addr1 = (CELL*)stack_pop(vm);
  CELL* addr2;
  addr2 = (CELL*)realloc(addr1, bytes);
  stack_push(vm, (CELL)addr2);
}

V query_malloc(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_MALLOC);
}

V io_malloc(NgaState *vm) {
  int i = stack_pop(vm);
  switch (i) {
    case 0: malloc_allocate(vm); return;
    case 1: malloc_free(vm); return;
    case 2: malloc_store(vm); return;
    case 3: malloc_fetch(vm); return;
    case 4: malloc_realloc(vm); return;
  }
  stack_push(vm, -1);
}
#endif
#endif
