/*---------------------------------------------------------------------
  Copyright (c) 2008 - 2022, Charles Childers

  Portions are based on Ngaro, which was additionally copyright
  by the following:

  Copyright (c) 2009 - 2010, Luke Parrish
  Copyright (c) 2010,        Marc Simpson
  Copyright (c) 2010,        Jay Skeer
  Copyright (c) 2011,        Kenneth Keating
  ---------------------------------------------------------------------*/

#ifdef ENABLE_FFI
#include <dlfcn.h>

typedef void (*External)(void *);

void *handles[32];
External funcs[32000];
int nlibs, nffi;

void open_library(NgaState *vm) {
  handles[nlibs] = dlopen(string_extract(vm, stack_pop(vm)), RTLD_LAZY);
  stack_push(vm, nlibs);
  nlibs++;
}

void map_symbol(NgaState *vm) {
  int h;
  h = stack_pop(vm);
  char *s = string_extract(vm, stack_pop(vm));
  funcs[nffi] = dlsym(handles[h], s);
  stack_push(vm, nffi);
  nffi++;
}

void invoke(NgaState *vm) {
  funcs[stack_pop(vm)](vm);
}

void io_ffi(NgaState *vm) {
  switch (stack_pop(vm)) {
    case 0: open_library(vm); break;
    case 1: map_symbol(vm); break;
    case 2: invoke(vm); break;
  }
}

void query_ffi(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 8100);  /* device type 8100 */
}
#endif
