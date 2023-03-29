/*---------------------------------------------------------------------
  Copyright (c) 2008 - 2022, Charles Childers

  Portions are based on Ngaro, which was additionally copyright
  by the following:

  Copyright (c) 2009 - 2010, Luke Parrish
  Copyright (c) 2010,        Marc Simpson
  Copyright (c) 2010,        Jay Skeer
  Copyright (c) 2011,        Kenneth Keating
  ---------------------------------------------------------------------*/

/* Time and Date Functions --------------------------------------------*/
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