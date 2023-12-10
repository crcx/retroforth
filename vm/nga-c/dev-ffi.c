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


#ifdef ENABLE_FFI
#include <dlfcn.h>

typedef void (*External)(void *);

V *handles[32];
External funcs[32000];
int nlibs, nffi;

V open_library(NgaState *vm) {
  handles[nlibs] = dlopen(string_extract(vm, stack_pop(vm)), RTLD_LAZY);
  stack_push(vm, nlibs);
  nlibs++;
}

V map_symbol(NgaState *vm) {
  int h;
  h = stack_pop(vm);
  char *s = string_extract(vm, stack_pop(vm));
  funcs[nffi] = dlsym(handles[h], s);
  stack_push(vm, nffi);
  nffi++;
}

V invoke(NgaState *vm) {
  funcs[stack_pop(vm)](vm);
}

V io_ffi(NgaState *vm) {
  switch (stack_pop(vm)) {
    case 0: open_library(vm); break;
    case 1: map_symbol(vm); break;
    case 2: invoke(vm); break;
  }
}

V query_ffi(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_FFI);
}
#endif
