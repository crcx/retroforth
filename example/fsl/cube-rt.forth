\ Cube root of real number by Newton's method
\ ANS compatible version V1.2  10/6/1994

\ Forth Scientific Library Algorithm #5

\ This code conforms with ANS requiring:
\      The FLOAT and FLOAT EXT word sets
\ Non STANDARD words
\      : FTUCK  ( F: x y -- y x y)     FSWAP FOVER ;
\      : F2*    ( F: x   -- 2x )       FDUP F+     ;
\      : F**2                          FDUP F*     ;


\     (c) Copyright 1994  Julian V. Noble.     Permission is granted
\     by the author to use this software for any application provided
\     the copyright notice is preserved.


3  S>D  D>F   FCONSTANT F=3

: X'       ( F: N x -- x')
           FTUCK   F**2  F/  FSWAP  F2*  F+  F=3  F/  ;

\ The magic number 1E-8 needs no change, even when extended (80-bit) precision
\ is needed.
: CONVERGED?    ( F: x' x x' --) ( -- f)
        F-   FOVER   F/   FABS   1.0E-8  F<   ;

: FCBRT    ( F: N -- N^1/3)
        FDUP  F0<  FABS                 ( F: -- |N|)  ( -- f)
        FDUP  FSQRT                     ( F: -- N x0 )
        BEGIN    FOVER FOVER  X'   FTUCK   CONVERGED?   UNTIL
        X'   IF   FNEGATE   THEN  ;

~~~
:x' (f:nx-X)
  f:tuck f:dup f:* f:/ f:swap f:dup f:+ f:+ .3 f:/ ;

:converged? (f:XxX-) (-f)
  f:- f:over f:/ f:abs .1.0e-8 f:lt? ;

:fcbrt (f:n-[n^1/3])
  f:dup .0 f:lt? f:abs
  f:dup f:sqrt
  [ f:over f:over x' f:tuck converged? ] until
  x' n:-zero? [ f:negate ] if ;
~~~

~~~
.9 fcbrt f:put nl
~~~

