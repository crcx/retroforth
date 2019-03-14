\ elip     Complete Elliptic Integral         ACM Algorithm #149

\ Forth Scientific Library Algorithm #2

\ Evaluates the Complete Elliptic Integral,
\     Elip[a, b] = int_0^{\pi/2} 1/Sqrt{a^2 cos^2(t) + b^2 sin^2(t)} dt

\ This function can be used to evaluate the complete elliptic integral
\ of the first kind, by using the relation K[m] = a Elip[a,b],  m = 1 - b^2/a^2
 
\ This code conforms with ANS requiring:
\      1. The Floating-Point word set
\      2. The immediate word '%' which takes the next token
\         and converts it to a floating-point literal
\      3. The FCONSTANT PI (3.1415926536...)
\ 
\ Both a recursive form and an iterative form are given, but because of the
\ large stack consumption the recursive form is probably not of much
\ practical use.

\ Caution: this code can potentially go into an endless loop
\          for certain values of the parameters.

\ Collected Algorithms from ACM, Volume 1 Algorithms 1-220,
\ 1980; Association for Computing Machinery Inc., New York,
\ ISBN 0-89791-017-6

\ (c) Copyright 1994 Everett F. Carter.  Permission is granted by the
\ author to use this software for any application provided the
\ copyright notice is preserved.

CR .( ELIP     V1.2                  21 September 1994   EFC )


: elip1 ( --, f: a b -- elip[a,b] )     \ recursive form

     FOVER FOVER FOVER F- FABS
     FSWAP % 1.0e-8 F*
     F< IF
          FDROP
          pi % 2.0 F/ FSWAP F/
        ELSE
          FOVER FOVER F+ % 2.0 F/
          FROT  FROT  F* FSQRT
          RECURSE
        THEN
;

: elip2 ( --, f: a b -- elip[a,b] )     \ nonrecursive version

    BEGIN
      FOVER FOVER F+ % 2.0 F/
      FROT  FROT  F* FSQRT

      FOVER FOVER FOVER F- FABS
      FSWAP % 1.0e-8 F*
   F< UNTIL

      FDROP

      pi % 2.0 F/ FSWAP F/

;


\ test driver,  calculates the complete elliptic integral of the first
\ kind (K(m)) using the relation: K[m] = a Elip[a,b], m = 1 - b^2/a^2
\ compare with Abramowitz & Stegun, Handbook of Mathematical Functions,
\ Table 17.1

: elip_test ( -- )
        CR
        ."  m     K(m) exact    a Elip1[a,b]    a Elip2[a,b] " CR

       ." 0.0    1.57079633   "
       % 1000.0 % 1000.0 elip1 % 1000.0 F* F. ."     "
       % 1000.0 % 1000.0 elip2 % 1000.0 F* F. CR

      ." 0.44   1.80632756   "
      % 400.0 % 299.33259 elip1 % 400.0 F* F. ."     "
      % 400.0 % 299.33259 elip2 % 400.0 F* F. CR

      ." 0.75   2.15651565   "
      % 1000.0 % 500.0 elip1 % 1000.0 F* F. ."     "
      % 1000.0 % 500.0 elip2 % 1000.0 F* F. CR

      ." 0.96   3.01611249   "
      % 500.0 % 100.0 elip1 % 500.0 F* F.  ."     "
      % 500.0 % 100.0 elip2 % 500.0 F* F. CR


;

# The Code

~~~
:elip1 (-,f:ab-elip[a,b])
  f:over f:over f:over f:- f:abs
  f:swap .1.0e-8 f:*
  f:lt? [ f:drop
          f:PI .2.0 f:/ f:swap f:/
        ]
        [ f:over f:over f:+ .2.0 f:/
          f:rot  f:rot  f:* f:sqrt
          elip1
        ] choose
;
~~~

# Test

```
.1000 .1000 elip1 .1000 f:* f:put nl
```
