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


/*
| 001 | Data Stack Underflow     |
| 002 | Data Stack Overflow      |
| 003 | Address Stack Underflow  |
| 004 | Address Stack Overflow   |
| 005 | Invalid Memory Access    |
| 006 | Division by Zero         |
*/

#ifdef ENABLE_ERROR
V execute(NgaState *vm, CELL cell);

V handle_error(NgaState *vm, CELL error) {
  CELL saved_ip = vm->cpu[vm->active].ip;
  if (vm->ErrorHandlers[error] != 0) {
    printf("\nHandling %lld\n", (long long)error);
    execute(vm, vm->ErrorHandlers[error]);
  }
  vm->cpu[vm->active].ip = saved_ip;
}

V register_error_handler(NgaState *vm) {
  CELL ErrorID = stack_pop(vm);
  CELL ErrorHandler = stack_pop(vm);
  vm->ErrorHandlers[ErrorID] = ErrorHandler;
  printf("Assigned %lld to %lld\n", (long long)ErrorID, (long long)ErrorHandler);
}

V io_error(NgaState *vm) {
  switch (stack_pop(vm)) {
    case 0: register_error_handler(vm); break;
    default: break;
  }
}

V query_error(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_ERROR);
}

#endif
