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

#define MAX_FFI_LIBRARIES 32
#define MAX_FFI_FUNCTIONS 32000

V *handles[MAX_FFI_LIBRARIES];
External funcs[MAX_FFI_FUNCTIONS];
int nlibs, nffi;

V initialize_ffi(NgaState *vm) {
  (void)vm;
  nlibs = 0;
  nffi = 0;
}

V ffi_error(NgaState *vm, const char *message) {
  printf("\nERROR (nga/ffi): %s\n", message);
  ACTIVE.ip = IMAGE_SIZE;
  ACTIVE.rp = 0;
}

V open_library(NgaState *vm) {
  V *handle;
  if (nlibs >= MAX_FFI_LIBRARIES) {
    ffi_error(vm, "Too many open libraries");
    return;
  }
  handle = dlopen(string_extract(vm, stack_pop(vm)), RTLD_LAZY);
  if (handle == NULL) {
    stack_push(vm, -1);
    return;
  }
  handles[nlibs] = handle;
  stack_push(vm, nlibs);
  nlibs++;
}

V map_symbol(NgaState *vm) {
  int h;
  h = stack_pop(vm);
  if (h < 0 || h >= nlibs || handles[h] == NULL) {
    ffi_error(vm, "Invalid library handle");
    return;
  }
  if (nffi >= MAX_FFI_FUNCTIONS) {
    ffi_error(vm, "Too many mapped symbols");
    return;
  }
  char *s = string_extract(vm, stack_pop(vm));
  funcs[nffi] = dlsym(handles[h], s);
  if (funcs[nffi] == NULL) {
    stack_push(vm, -1);
    return;
  }
  stack_push(vm, nffi);
  nffi++;
}

V invoke(NgaState *vm) {
  CELL function = stack_pop(vm);
  if (function < 0 || function >= nffi || funcs[function] == NULL) {
    ffi_error(vm, "Invalid function handle");
    return;
  }
  funcs[function](vm);
}

V io_ffi(NgaState *vm) {
  switch (stack_pop(vm)) {
    case 0: open_library(vm); break;
    case 1: map_symbol(vm); break;
    case 2: invoke(vm); break;
    default: ffi_error(vm, "Invalid FFI action"); break;
  }
}

V query_ffi(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_FFI);
}
#endif
