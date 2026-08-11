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

#include "retro.h"

#ifdef ENABLE_CLOCK
time_t current_time;

V clock_time(NgaState *vm) {
  stack_push(vm, (CELL)time(NULL));
}

V clock_day(NgaState *vm) {
  stack_push(vm, (CELL)localtime(&current_time)->tm_mday);
}

V clock_month(NgaState *vm) {
  stack_push(vm, (CELL)localtime(&current_time)->tm_mon + 1);
}

V clock_year(NgaState *vm) {
  stack_push(vm, (CELL)localtime(&current_time)->tm_year + 1900);
}

V clock_hour(NgaState *vm) {
  stack_push(vm, (CELL)localtime(&current_time)->tm_hour);
}

V clock_minute(NgaState *vm) {
  stack_push(vm, (CELL)localtime(&current_time)->tm_min);
}

V clock_second(NgaState *vm) {
  stack_push(vm, (CELL)localtime(&current_time)->tm_sec);
}

V clock_day_utc(NgaState *vm) {
  stack_push(vm, (CELL)gmtime(&current_time)->tm_mday);
}

V clock_month_utc(NgaState *vm) {
  stack_push(vm, (CELL)gmtime(&current_time)->tm_mon + 1);
}

V clock_year_utc(NgaState *vm) {
  stack_push(vm, (CELL)gmtime(&current_time)->tm_year + 1900);
}

V clock_hour_utc(NgaState *vm) {
  stack_push(vm, (CELL)gmtime(&current_time)->tm_hour);
}

V clock_minute_utc(NgaState *vm) {
  stack_push(vm, (CELL)gmtime(&current_time)->tm_min);
}

V clock_second_utc(NgaState *vm) {
  stack_push(vm, (CELL)gmtime(&current_time)->tm_sec);
}

Handler ClockActions[] = {
  clock_time,     clock_day,        clock_month,
  clock_year,     clock_hour,       clock_minute,
  clock_second,
  clock_day_utc,  clock_month_utc,  clock_year_utc,
  clock_hour_utc, clock_minute_utc, clock_second_utc
};

V query_clock(NgaState *vm) {
  stack_push(vm, 0);
  stack_push(vm, DEVICE_CLOCK);
}

V invalid_clock_action(NgaState *vm, CELL action) {
  printf("\nERROR (nga/clock): Invalid clock action %lld\n", (long long)action);
  ACTIVE.ip = IMAGE_SIZE;
  ACTIVE.rp = 0;
}

V io_clock(NgaState *vm) {
  CELL action = stack_pop(vm);
  CELL actions = sizeof(ClockActions) / sizeof(ClockActions[0]);
  current_time = time(NULL);
  if (action >= 0 && action < actions) {
    ClockActions[action](vm);
  } else {
    invalid_clock_action(vm, action);
  }
}
#endif
