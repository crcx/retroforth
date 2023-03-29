/*---------------------------------------------------------------------
  Copyright (c) 2008 - 2022, Charles Childers

  Portions are based on Ngaro, which was additionally copyright
  by the following:

  Copyright (c) 2009 - 2010, Luke Parrish
  Copyright (c) 2010,        Marc Simpson
  Copyright (c) 2010,        Jay Skeer
  Copyright (c) 2011,        Kenneth Keating
  ---------------------------------------------------------------------*/

/* Random Number Generator --------------------------------------------*/
#ifdef ENABLE_RNG
void io_rng(NgaState *vm) {
  int64_t r = 0;
  char buffer[8];
  int i;
  ssize_t ignore;
  int fd = open("/dev/urandom", O_RDONLY);
  ignore = read(fd, buffer, 8);
  close(fd);
  for(i = 0; i < 8; ++i) {
    r = r << 8;
    r += ((int64_t)buffer[i] & 0xFF);
  }
#ifndef BIT64
  stack_push(vm, (CELL)abs((CELL)r));
#else
  stack_push(vm, (CELL)llabs((CELL)r));
#endif
}

void query_rng(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 10);
}
#endif
