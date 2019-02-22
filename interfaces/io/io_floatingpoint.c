/* RETRO ------------------------------------------------------
  A personal, minimalistic forth
  Copyright (c) 2016 - 2019 Charles Childers
  ---------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

typedef void (*Handler)(void);

/*---------------------------------------------------------------------
  First, a few things that relate to the Nga virtual machine that RETRO
  runs on.
  ---------------------------------------------------------------------*/

#define CELL         int32_t      /* Cell size (32 bit, signed integer */
#define IMAGE_SIZE   524288 * 48  /* Amount of RAM. 12MiB by default.  */
#define ADDRESSES    2048         /* Depth of address stack            */
#define STACK_DEPTH  512          /* Depth of data stack               */

extern CELL sp, rp, ip;             /* Stack & instruction pointers    */
extern CELL data[STACK_DEPTH];      /* The data stack                  */
extern CELL address[ADDRESSES];     /* The address stack               */
extern CELL memory[IMAGE_SIZE + 1]; /* The memory for the image        */
extern char string_data[];

#define TOS  data[sp]             /* Shortcut for top item on stack    */
#define NOS  data[sp-1]           /* Shortcut for second item on stack */
#define TORS address[rp]          /* Shortcut for top item on address stack */

/*---------------------------------------------------------------------
  Function prototypes.
  ---------------------------------------------------------------------*/

CELL stack_pop();
void stack_push(CELL value);
char *string_extract(CELL at);
CELL string_inject(char *str, CELL buffer);



/*---------------------------------------------------------------------
  I have a stack of floating point values ("floats") and a stack
  pointer (`fsp`).  
  ---------------------------------------------------------------------*/

double Floats[8192];
CELL fsp;

double AFloats[8192];
CELL afsp;


/*---------------------------------------------------------------------
  The first two functions push a float to the stack and pop a value off
  the stack.
  ---------------------------------------------------------------------*/

void float_push(double value) {
    fsp++;
    Floats[fsp] = value;
}

double float_pop() {
    fsp--;
    return Floats[fsp + 1];
}

void float_to_alt() {
  afsp++;
  AFloats[afsp] = float_pop();
}

void float_from_alt() {
  float_push(AFloats[afsp]);
  afsp--;
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
  float_from_number,
  float_from_string,
  float_to_number,
  float_to_string,
  float_add,
  float_sub,
  float_mul,
  float_div,
  float_floor,
  float_ceil,
  float_sqrt,
  float_eq,
  float_neq,
  float_lt,
  float_gt,
  float_depth,
  float_dup,
  float_drop,
  float_swap,
  float_log,
  float_pow,
  float_sin,
  float_tan,
  float_cos,
  float_asin,
  float_acos,
  float_atan,
  float_to_alt,
  float_from_alt,
  float_adepth,
};

void io_floatingpoint_query() {
  stack_push(1);
  stack_push(2);
}

void io_floatingpoint_handler() {
  FloatHandlers[stack_pop()]();
}
