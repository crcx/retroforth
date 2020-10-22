/*---------------------------------------------------------------------
  Random Number Generator
  ---------------------------------------------------------------------*/

void io_random() {
  int64_t r = 0;
  char buffer[8];
  int i;
  int fd = open("/dev/urandom", O_RDONLY);
  read(fd, buffer, 8);
  close(fd);
  for(i = 0; i < 8; ++i) {
    r = r << 8;
    r += ((int64_t)buffer[i] & 0xFF);
  }
#ifndef BIT64
  stack_push((CELL)abs((CELL)r));
#else
  stack_push(llabs(r));
#endif
}

void io_random_query() {
  stack_push(0);
  stack_push(10);
}
