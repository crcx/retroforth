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


/* Multi Core Support ------------------------------------------------ */
#ifdef ENABLE_MULTICORE
void init_core(NgaState *vm, CELL x) {
  int y;
  vm->cpu[x].sp = 0;
  vm->cpu[x].rp = 0;
  vm->cpu[x].ip = 0;
  vm->cpu[x].active = 0;
  vm->cpu[x].u = 0;
  for (y = 0; y < STACK_DEPTH; y++) { vm->cpu[x].data[y] = 0; };
  for (y = 0; y < ADDRESSES; y++) { vm->cpu[x].address[y] = 0; };
  for (y = 0; y < 24; y++) { vm->cpu[x].registers[y] = 0; };
}

void start_core(NgaState *vm, CELL x, CELL ip) {
  vm->cpu[x].ip = ip;
  vm->cpu[x].rp = 1;
  vm->cpu[x].active = -1;
}

void pause_core(NgaState *vm, CELL x) {
  vm->cpu[x].active = 0;
}

void resume_core(NgaState *vm, CELL x) {
  vm->cpu[x].active = -1;
}

void switch_core(NgaState *vm) {
  vm->active += 1;
  if (vm->active >= CORES) { vm->active = 0; }
  if (!vm->cpu[vm->active].active) { switch_core(vm); }
}

void io_multicore(NgaState *vm) {
  int x, y, z;
  x = stack_pop(vm);
  switch(x) {
    case 0: y = stack_pop(vm);
            init_core(vm, y);
            break;
    case 1: y = stack_pop(vm);
            z = stack_pop(vm);
            start_core(vm, y, z);
            break;
    case 2: y = stack_pop(vm);
            pause_core(vm, y);
            break;
    case 3: pause_core(vm, vm->active);
            break;
    case 4: y = stack_pop(vm);
            resume_core(vm, y);
            break;
    case 5: y = stack_pop(vm);
            stack_push(vm, vm->cpu[vm->active].registers[y]);
            break;
    case 6: y = stack_pop(vm);
            z = stack_pop(vm);
            vm->cpu[vm->active].registers[y] = z;
            break;
  }
}

void query_multicore(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_MULTICORE);
}
#endif
