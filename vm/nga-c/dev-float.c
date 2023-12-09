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


#ifdef ENABLE_FLOATS
#include <math.h>

/* Floating Point ---------------------------------------------------- */

void float_guard(NgaState *vm) {
  if (vm->fsp < 0 || vm->fsp > 255) {
    printf("\nERROR (nga/float_guard): Float Stack Limits Exceeded!\n");
    printf("At %lld, fsp = %lld\n", (long long)vm->cpu[vm->active].ip, (long long)vm->fsp);
    exit(1);
  }
  if (vm->afsp < 0 || vm->afsp > 255) {
    printf("\nERROR (nga/float_guard): Alternate Float Stack Limits Exceeded!\n");
    printf("At %lld, afsp = %lld\n", (long long)vm->cpu[vm->active].ip, (long long)vm->afsp);
    exit(1);
  }
}

/*---------------------------------------------------------------------
  The first two functions push a float to the stack and pop a value off
  the stack.
  ---------------------------------------------------------------------*/

void float_push(NgaState *vm, double value) {
  vm->fsp++;
  float_guard(vm);
  vm->Floats[vm->fsp] = value;
}

double float_pop(NgaState *vm) {
  vm->fsp--;
  float_guard(vm);
  return vm->Floats[vm->fsp + 1];
}

void float_to_alt(NgaState *vm) {
  vm->afsp++;
  float_guard(vm);
  vm->AFloats[vm->afsp] = float_pop(vm);
}

void float_from_alt(NgaState *vm) {
  float_push(vm, vm->AFloats[vm->afsp]);
  vm->afsp--;
  float_guard(vm);
}


/*---------------------------------------------------------------------
  RETRO operates on 32-bit signed integer values. This function just
  pops a number from the data stack, casts it to a float, and pushes it
  to the float stack.
  ---------------------------------------------------------------------*/
void float_from_number(NgaState *vm) {
  float_push(vm, (double)stack_pop(vm));
}


/*---------------------------------------------------------------------
  To get a float from a string in the image, I provide this function.
  I cheat: using `atof()` takes care of the details, so I don't have
  to.
  ---------------------------------------------------------------------*/
void float_from_string(NgaState *vm) {
  float_push(vm, atof(string_extract(vm, stack_pop(vm))));
}


/*---------------------------------------------------------------------
  Converting a floating point into a string is slightly more work. Here
  I pass it off to `snprintf()` to deal with.
  ---------------------------------------------------------------------*/
void float_to_string(NgaState *vm) {
  snprintf(vm->string_data, 8192, "%f", float_pop(vm));
  string_inject(vm, vm->string_data, stack_pop(vm));
}


/*---------------------------------------------------------------------
  Converting a floating point back into a standard number requires a
  little care due to the signed nature. This makes adjustments for the
  max & min value, and then casts (rounding) the float back to a normal
  number.
  ---------------------------------------------------------------------*/

void float_to_number(NgaState *vm) {
  double a = float_pop(vm);
  if (a > 2147483647)
    a = 2147483647;
  if (a < -2147483648)
    a = -2147483648;
  stack_push(vm, (CELL)round(a));
}

void float_add(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, a+b);
}

void float_sub(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, b-a);
}

void float_mul(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, a*b);
}

void float_div(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, b/a);
}

void float_floor(NgaState *vm) {
  float_push(vm, floor(float_pop(vm)));
}

void float_ceil(NgaState *vm) {
  float_push(vm, ceil(float_pop(vm)));
}

void float_eq(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  if (a == b)
    stack_push(vm, -1);
  else
    stack_push(vm, 0);
}

void float_neq(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  if (a != b)
    stack_push(vm, -1);
  else
    stack_push(vm, 0);
}

void float_lt(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  if (b < a)
    stack_push(vm, -1);
  else
    stack_push(vm, 0);
}

void float_gt(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  if (b > a)
    stack_push(vm, -1);
  else
    stack_push(vm, 0);
}

void float_depth(NgaState *vm) {
  stack_push(vm, vm->fsp);
}

void float_adepth(NgaState *vm) {
  stack_push(vm, vm->afsp);
}

void float_dup(NgaState *vm) {
  double a = float_pop(vm);
  float_push(vm, a);
  float_push(vm, a);
}

void float_drop(NgaState *vm) {
  float_pop(vm);
}

void float_swap(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, a);
  float_push(vm, b);
}

void float_log(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, log(b) / log(a));
}

void float_sqrt(NgaState *vm) {
  float_push(vm, sqrt(float_pop(vm)));
}

void float_pow(NgaState *vm) {
  double a = float_pop(vm);
  double b = float_pop(vm);
  float_push(vm, pow(b, a));
}

void float_sin(NgaState *vm) {
  float_push(vm, sin(float_pop(vm)));
}

void float_cos(NgaState *vm) {
  float_push(vm, cos(float_pop(vm)));
}

void float_tan(NgaState *vm) {
  float_push(vm, tan(float_pop(vm)));
}

void float_asin(NgaState *vm) {
  float_push(vm, asin(float_pop(vm)));
}

void float_acos(NgaState *vm) {
  float_push(vm, acos(float_pop(vm)));
}

void float_atan(NgaState *vm) {
  float_push(vm, atan(float_pop(vm)));
}


/*---------------------------------------------------------------------
  With this finally done, I implement the FPU instructions.
  ---------------------------------------------------------------------*/
Handler FloatHandlers[] = {
  float_from_number,  float_from_string,
  float_to_number,    float_to_string,
  float_add,      float_sub,     float_mul,   float_div,
  float_floor,    float_ceil,    float_sqrt,  float_eq,
  float_neq,      float_lt,      float_gt,    float_depth,
  float_dup,      float_drop,    float_swap,  float_log,
  float_pow,      float_sin,     float_tan,   float_cos,
  float_asin,     float_acos,    float_atan,  float_to_alt,
  float_from_alt, float_adepth,
};

void query_floatingpoint(NgaState *vm) {
  stack_push(vm, 1);
  stack_push(vm, DEVICE_FLOATS);
}

void io_floatingpoint(NgaState *vm) {
  FloatHandlers[stack_pop(vm)](vm);
}
#endif
