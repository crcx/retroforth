#include "retroforth.h"

CELL stack_pop(NgaState *vm) {
  vm->cpu[vm->active].sp--;
  return vm->cpu[vm->active].data[vm->cpu[vm->active].sp + 1];
}
void stack_push(NgaState *vm, CELL value) {
  vm->cpu[vm->active].sp++;
  vm->cpu[vm->active].data[vm->cpu[vm->active].sp] = value;
}

void fortytwo(NgaState *retro) {
  stack_push(retro, 42);
}
