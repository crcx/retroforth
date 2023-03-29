/*---------------------------------------------------------------------
  Copyright (c) 2008 - 2022, Charles Childers

  Portions are based on Ngaro, which was additionally copyright
  by the following:

  Copyright (c) 2009 - 2010, Luke Parrish
  Copyright (c) 2010,        Marc Simpson
  Copyright (c) 2010,        Jay Skeer
  Copyright (c) 2011,        Kenneth Keating
  ---------------------------------------------------------------------*/

#ifdef ENABLE_MALLOC
#ifdef BIT64
typedef union {
  void* val;
  struct {
    CELL msw;
    CELL lsw;
  };
} double_cell;

void double_add(NgaState *vm) {
  double_cell a;
  double_cell b;
  double_cell c;
  b.msw = stack_pop(vm);
  b.lsw = stack_pop(vm);
  a.msw = stack_pop(vm);
  a.lsw = stack_pop(vm);
}

void double_sub(NgaState *vm) {
}

void double_mul(NgaState *vm) {
}

void double_divmod(NgaState *vm) {
}

void malloc_allocate(NgaState *vm) {
  stack_push(vm, (CELL)malloc(stack_pop(vm)));
}

void malloc_free(NgaState *vm) {
  free((CELL*)stack_pop(vm));
}

void malloc_store(NgaState *vm) {
  CELL value = stack_pop(vm);
  double_cell addr;
  *(CELL *) stack_pop(vm) = value;
}

void malloc_fetch(NgaState *vm) {
  double_cell addr;
  CELL value = *(CELL *)stack_pop(vm);
  stack_push(vm, value);
}

void malloc_realloc(NgaState *vm) {
  CELL bytes = stack_pop(vm);
  CELL* addr1;
  addr1 = (CELL*)stack_pop(vm);
  CELL* addr2;
  addr2 = (CELL*)realloc(addr1, bytes);
  stack_push(vm, (CELL)addr2);
}

void query_malloc(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 15);
}

void io_malloc(NgaState *vm) {
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