\ expint     Real Exponential Integral         ACM Algorithm #20

\ Forth Scientific Library Algorithm #1

\ Evaluates the Real Exponential Integral,
\     E1(x) = - Ei(-x) =   int_x^\infty exp^{-u}/u du      for x > 0
\ using a rational approximation

\ This code conforms with ANS requiring:
\      1. The Floating-Point word set
\      2. The immediate word '%' which takes the next token
\         and converts it to a floating-point literal
\ 

\ Collected Algorithms from ACM, Volume 1 Algorithms 1-220,
\ 1980; Association for Computing Machinery Inc., New York,
\ ISBN 0-89791-017-6

\ (c) Copyright 1994 Everett F. Carter.  Permission is granted by the
\ author to use this software for any application provided the
\ copyright notice is preserved.


CR .( EXPINT     V1.1                  21 September 1994   EFC )


: expint ( --, f: x -- expint[x] )
        FDUP
        % 1.0 F< IF
                    FDUP % 0.00107857 F* % 0.00976004 F-
                    FOVER F*
                    % 0.05519968 F+
                    FOVER F*
                    % 0.24991055 F-
                    FOVER F*
                    % 0.99999193 F+
                    FOVER F*
                    % 0.57721566 F-
                    FSWAP FLN F-
                ELSE
                    FDUP % 8.5733287401 F+
                    FOVER F*
                    % 18.059016973 F+
                    FOVER F*
                    % 8.6347608925 F+
                    FOVER F*
                    % 0.2677737343 F+

                    FOVER
                    FDUP % 9.5733223454 F+
                    FOVER F*
                    % 25.6329561486 F+
                    FOVER F*
                    % 21.0996530827 F+
                    FOVER F*
                    % 3.9584969228 F+

                    FSWAP FDROP
                    F/
                    FOVER F/
                    FSWAP % -1.0 F* FEXP
                    F*

                THEN
;


\ test code generates a small table of selected E1 values.
\ most comparison values are from Abramowitz & Stegun,
\ Handbook of Mathematical Functions, Table 5.1

: expint_test ( -- )

        CR
        ."   x    E1(x) exact      ExpInt[x] " CR


      ."  0.5   0.5597736      "
      % 0.5 expint  F. CR

      ."  1.0   0.2193839      "
      % 1.0 expint   F. CR

      ."  2.0   0.0489005      "
      % 2.0 expint    F. CR

      ."  5.0   0.001148296    "
      % 5.0 expint    F. CR

      ." 10.0   0.4156969e-5   "
      % 10.0 expint   F. CR

;

# The Code

~~~
:expint (-,f:x-expint[x])
  f:dup
  .1.0 f:lt? [
    f:dup .0.00107857 f:* .0.00976004 f:-
    f:over f:*
    .0.05519968 f:+
    f:over f:*
    .0.24991055 f:-
    f:over f:*
    .0.99999193 f:+
    f:over f:*
    .0.57721566 f:-
    f:swap f:E f:log f:-
  ] [
    f:dup .8.5733287401 f:+
    f:over f:*
    .18.059016973 f:+
    f:over f:*
    .8.6347608925 f:+
    f:over f:*
    .0.2677737343 f:+

    f:over
    f:dup .9.5733223454 f:+
    f:over f:*
    .25.6329561486 f:+
    f:over f:*
    .21.0996530827 f:+
    f:over f:*
    .3.9584969228 f:+

    f:swap f:drop
    f:/
    f:over f:/
    f:swap .-1.0 f:* f:E f:swap f:power
    f:*
  ] choose
;
~~~

# Tests

```
.0.5 expint f:put nl
.1.0 expint f:put nl
.2.0 expint f:put nl
.5.0 expint f:put nl
.10.0 expint f:put nl
```
