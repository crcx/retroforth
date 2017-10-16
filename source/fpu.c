#include <math.h>

double Floats[8192];
CELL fsp;

void float_push(double value) {
    fsp++;
    Floats[fsp] = value;
}

double float_pop() {
    fsp--;
    return Floats[fsp + 1];
}

void float_from_number() {
    float_push((double)stack_pop());
}

void float_from_string() {
    double a = atof(string_extract(stack_pop()));
    float_push(a);
}

void float_to_string() {
    snprintf(string_data, 8192, "%f", float_pop());
    string_inject(string_data, stack_pop());
}

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
    double a = float_pop();
    float_push(floor(a));
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

void float_pow() {
    double a = float_pop();
    double b = float_pop();
    float_push(pow(b, a));
}

void float_to_number() {
    double a = float_pop();
    if (a > 2147483647)
      a = 2147483647;
    if (a < -2147483648)
      a = -2147483648;
    a = round(a);
    stack_push((CELL)a);
}

void ngaFloatingPointUnit() {
    switch (stack_pop()) {
        case 0: float_from_number();
            break;
        case 1: float_from_string();
            break;
        case 2: float_to_string();
            break;
        case 3: float_add();
            break;
        case 4: float_sub();
            break;
        case 5: float_mul();
            break;
        case 6: float_div();
            break;
        case 7: float_floor();
            break;
        case 8: float_eq();
            break;
        case 9: float_neq();
            break;
        case 10: float_lt();
            break;
        case 11: float_gt();
            break;
        case 12: float_depth();
            break;
        case 13: float_dup();
            break;
        case 14: float_drop();
            break;
        case 15: float_swap();
            break;
        case 16: float_log();
            break;
        case 17: float_pow();
            break;
        case 18: float_to_number();
            break;
        default: ;
    }
}
