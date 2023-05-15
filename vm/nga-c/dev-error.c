/***************************************************************
  Copyright (c) Charles Childers
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

void register_error_handler(NgaState *vm) {
  CELL ErrorID = stack_pop(vm);
  CELL ErrorHandler = stack_pop(vm);
  vm->ErrorHandlers[ErrorID] = ErrorHandler;
}

void io_error(NgaState *vm) {
  switch (stack_pop(vm)) {
    case 0: register_error_handler(vm);
    default: break;
  }
}

void query_err(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 1234);
}

#endif
