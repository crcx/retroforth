/*---------------------------------------------------------------------
  Time and Date Functions
  ---------------------------------------------------------------------*/

#include <time.h>

void unix_time() {
  stack_push((CELL)time(NULL));
}

void unix_time_day() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_mday);
}

void unix_time_month() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_mon + 1);
}

void unix_time_year() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_year + 1900);
}

void unix_time_hour() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_hour);
}

void unix_time_minute() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_min);
}

void unix_time_second() {
  time_t t = time(NULL);
  stack_push((CELL)localtime(&t)->tm_sec);
}

void unix_time_day_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_mday);
}

void unix_time_month_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_mon + 1);
}

void unix_time_year_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_year + 1900);
}

void unix_time_hour_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_hour);
}

void unix_time_minute_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_min);
}

void unix_time_second_utc() {
  time_t t = time(NULL);
  stack_push((CELL)gmtime(&t)->tm_sec);
}

Handler ClockActions[] = {
  unix_time,
  unix_time_day,      unix_time_month,      unix_time_year,
  unix_time_hour,     unix_time_minute,     unix_time_second,
  unix_time_day_utc,  unix_time_month_utc,  unix_time_year_utc,
  unix_time_hour_utc, unix_time_minute_utc, unix_time_second_utc
};

void io_clock_query() {
  stack_push(0);
  stack_push(5);
}

void io_clock_handler() {
  ClockActions[stack_pop()]();
}

