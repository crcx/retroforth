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

#ifdef ENABLE_CLOCK
void clock_time(NgaState *vm) {
  stack_push(vm, (CELL)time(NULL));
}

void clock_day(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)localtime(&t)->tm_mday);
}

void clock_month(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)localtime(&t)->tm_mon + 1);
}

void clock_year(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)localtime(&t)->tm_year + 1900);
}

void clock_hour(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)localtime(&t)->tm_hour);
}

void clock_minute(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)localtime(&t)->tm_min);
}

void clock_second(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)localtime(&t)->tm_sec);
}

void clock_day_utc(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)gmtime(&t)->tm_mday);
}

void clock_month_utc(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)gmtime(&t)->tm_mon + 1);
}

void clock_year_utc(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)gmtime(&t)->tm_year + 1900);
}

void clock_hour_utc(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)gmtime(&t)->tm_hour);
}

void clock_minute_utc(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)gmtime(&t)->tm_min);
}

void clock_second_utc(NgaState *vm) {
  time_t t = time(NULL);
  stack_push(vm, (CELL)gmtime(&t)->tm_sec);
}

Handler ClockActions[] = {
  clock_time,
  clock_day,      clock_month,      clock_year,
  clock_hour,     clock_minute,     clock_second,
  clock_day_utc,  clock_month_utc,  clock_year_utc,
  clock_hour_utc, clock_minute_utc, clock_second_utc
};

void query_clock(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, 5);
}

void io_clock(NgaState *vm) {
  ClockActions[stack_pop(vm)](vm);
}
#endif
