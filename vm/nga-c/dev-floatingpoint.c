/*---------------------------------------------------------------------
  Floating Point
  ---------------------------------------------------------------------*/

/*---------------------------------------------------------------------
  I have a stack of floating point values ("floats") and a stack
  pointer (`fsp`).  
  ---------------------------------------------------------------------*/

double Floats[256];
CELL fsp;

double AFloats[256];
CELL afsp;


/*---------------------------------------------------------------------
  The first two functions push a float to the stack and pop a value off
  the stack.
  ---------------------------------------------------------------------*/

void float_guard() {
#ifndef NOCHECKS
  if (fsp < 0 || fsp > 255) {
    printf("\nERROR (nga/float_guard): Float Stack Limits Exceeded!\n");
    printf("At %lld, fsp = %lld\n", (long long)ip, (long long)fsp);
    exit(1);
  }
  if (afsp < 0 || afsp > 255) {
    printf("\nERROR (nga/float_guard): 	Alternate Float Stack Limits Exceeded!\n");
    printf("At %lld, afsp = %lld\n", (long long)ip, (long long)afsp);
    exit(1);
  }
#endif
}

void float_push(double value) {
  fsp++;
  float_guard();
  Floats[fsp] = value;
}

double float_pop() {
  fsp--;
  float_guard();
  return Floats[fsp + 1];
}

void float_to_alt() {
  afsp++;
  float_guard();
  AFloats[afsp] = float_pop();
}

void float_from_alt() {
  float_push(AFloats[afsp]);
  afsp--;
  float_guard();
}


/*---------------------------------------------------------------------
  RETRO operates on 32-bit signed integer values. This function just
  pops a number from the data stack, casts it to a float, and pushes it
  to the float stack.
  ---------------------------------------------------------------------*/
void float_from_number() {
    float_push((double)stack_pop());
}


/*---------------------------------------------------------------------
  To get a float from a string in the image, I provide this function.
  I cheat: using `atof()` takes care of the details, so I don't have
  to.
  ---------------------------------------------------------------------*/
void float_from_string() {
    float_push(atof(string_extract(stack_pop())));
}


/*---------------------------------------------------------------------
  Converting a floating point into a string is slightly more work. Here
  I pass it off to `snprintf()` to deal with.
  ---------------------------------------------------------------------*/
void float_to_string() {
    snprintf(string_data, 8192, "%f", float_pop());
    string_inject(string_data, stack_pop());
}


/*---------------------------------------------------------------------
  Converting a floating point back into a standard number requires a
  little care due to the signed nature. This makes adjustments for the
  max & min value, and then casts (rounding) the float back to a normal
  number.
  ---------------------------------------------------------------------*/

void float_to_number() {
    double a = float_pop();
    if (a > 2147483647)
      a = 2147483647;
    if (a < -2147483648)
      a = -2147483648;
    stack_push((CELL)round(a));
}


/*---------------------------------------------------------------------
  Now I get to define a bunch of functions that operate on floats.
  These provide the basic math, and wrappers around functionality in
  libm.
  ---------------------------------------------------------------------*/

void float_add() {
    double a = float_pop();
    double b = float_pop();
    float_push(a+b);
}

void float_sub() {
    double a = float_pop();
    double b = float_pop();
    float_push(b-a);
}

void float_mul() {
    double a = float_pop();
    double b = float_pop();
    float_push(a*b);
}

void float_div() {
  double a = float_pop();
  double b = float_pop();
  float_push(b/a);
}

void float_floor() {
    float_push(floor(float_pop()));
}

void float_ceil() {
    float_push(ceil(float_pop()));
}

void float_eq() {
    double a = float_pop();
    double b = float_pop();
    if (a == b)
        stack_push(-1);
    else
        stack_push(0);
}

void float_neq() {
    double a = float_pop();
    double b = float_pop();
    if (a != b)
        stack_push(-1);
    else
        stack_push(0);
}

void float_lt() {
    double a = float_pop();
    double b = float_pop();
    if (b < a)
        stack_push(-1);
    else
        stack_push(0);
}

void float_gt() {
    double a = float_pop();
    double b = float_pop();
    if (b > a)
        stack_push(-1);
    else
        stack_push(0);
}

void float_depth() {
    stack_push(fsp);
}

void float_adepth() {
    stack_push(afsp);
}

void float_dup() {
    double a = float_pop();
    float_push(a);
    float_push(a);
}

void float_drop() {
    float_pop();
}

void float_swap() {
    double a = float_pop();
    double b = float_pop();
    float_push(a);
    float_push(b);
}

void float_log() {
    double a = float_pop();
    double b = float_pop();
    float_push(log(b) / log(a));
}

void float_sqrt() {
  float_push(sqrt(float_pop()));
}

void float_pow() {
    double a = float_pop();
    double b = float_pop();
    float_push(pow(b, a));
}

void float_sin() {
  float_push(sin(float_pop()));
}

void float_cos() {
  float_push(cos(float_pop()));
}

void float_tan() {
  float_push(tan(float_pop()));
}

void float_asin() {
  float_push(asin(float_pop()));
}

void float_acos() {
  float_push(acos(float_pop()));
}

void float_atan() {
  float_push(atan(float_pop()));
}


/*---------------------------------------------------------------------
  With this finally done, I implement the FPU instructions.
  ---------------------------------------------------------------------*/
Handler FloatHandlers[] = {
  float_from_number,  float_from_string,  float_to_number,  float_to_string,
  float_add,          float_sub,          float_mul,        float_div,
  float_floor,        float_ceil,         float_sqrt,       float_eq,
  float_neq,          float_lt,           float_gt,         float_depth,
  float_dup,          float_drop,         float_swap,       float_log,
  float_pow,          float_sin,          float_tan,        float_cos,
  float_asin,         float_acos,         float_atan,       float_to_alt,
  float_from_alt,     float_adepth,
};

void io_floatingpoint_query() {
  stack_push(1);
  stack_push(2);
}

void io_floatingpoint_handler() {
  FloatHandlers[stack_pop()]();
}
