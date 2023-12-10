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

#ifdef ENABLE_RNG
V io_rng(NgaState *vm) {
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

V query_rng(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_RNG);
}
#endif
