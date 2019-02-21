Here are words to encode floating point numbers into two cells.

Words in file FloatingPointEncoding.forth encode
floating point numbers, which are often  64-bit or 2-cell numbers,
into signed integers,   which are always 32-bit or 1-cell numbers.
The encoding causes loss in both precision and dynamic range.

Words defined in this file encodes floating point numbers into
two integers occupying two 32-bit cells.

Two different ways are implemented:

- `w1` used fixed point encoding.
   Splits a floating point number into the integer and the fraction
   parts to store them in two cells.
   This is faster with a narrower dynamic range.
- `w2` uses sqrt-encoding described in doc/SqrtEncoding.pdf .
   This is slower with a relatively wider dynamic range,
   but likely not as wide as the original floating point.

Since Retro's cells are 32-bit,
it is more convenient to handle floating point numbers
encoded into single cells than into pairs of cells.
For instance, consider using the `set:` class words for vectors.
Hence in many cases the words found by doing

  'u: d:words-with

are suitable.
However, sometimes a higher precision is desired at a higher cost.

~~~
:d:-found? (s-f) d:lookup n:zero? ;
:st (-) dump-stack nl f:dump-stack ;
's:abort d:-found? [ 'example/Abort.forth include ] if
~~~

Encode/decode words to secure dynamic range.
This is the essence of the method.

~~~
:f:encode.w2  (|f:n-n) f:dup f:sign f:abs f:sqrt n:to-float f:* ;
:f:-encode.w2 (|f:n-n) f:dup f:sign f:dup f:*    n:to-float f:* ;
~~~

```
.-12345.6789 f:encode.w2  st
             f:-encode.w2 st
```

Split encoded floating point numbers into the integer and the fraction parts.
The sign goes to the integer part.
The fractional part is that of the absolute value.

~~~
{{
  :f:shift9  .1.e-9 f:* ;
  :f:-shift9 .1.e9  f:* ;
---reveal---
  :f:split  (-|f:n-fi)_absFrac.-shift9_signedInt
    f:dup f:sign                      (s|f:_n
    f:abs f:dup f:floor f:swap f:over (s|f:_abs.int_abs_abs.int
    f:- f:-shift9                     (s|f:_abs.int_abs.frac.-shift9
    f:swap n:to-float f:*             (_|f:_abs.frac.-shift9_signedInt
    ;
  :f:-split (-|f:fi-n)
    f:dup f:sign f:abs                 (s|f:_abs.frac.-shift9_abs.int
    f:swap f:shift9 f:+ n:to-float f:* (_|f:_n 
    ;
}}
~~~

```
.-123456789.0987654321 f:split  st
                       f:-split st
```

From float to double cells w2 .
And its inverse.

~~~
:f:to-w1 (-fi|f:n-)_frac_sInt f:split f:to-number f:to-number swap     ;
:w1:to-f (fi-|f:-n)           swap    n:to-float  n:to-float  f:-split ;
:f:to-w2 (-fi|f:n-)_frac_sInt f:encode.w2  f:to-w1 ;
:w2:to-f (fi-|f:-n)   w1:to-f f:-encode.w2         ;
:st (-) dump-stack nl f:dump-stack ; 
  :f:sign (-n|f:a-) 
  f:dup .0 f:eq? [ #0 f:drop ] if; 
  .0 f:gt? [ #1 ] [ #-1 ] choose ; 
~~~

```
.-123456789.0987654321 f:to-w2 st
                       w2:to-f st
```

`f:var1` and `f:var2` take initial values.

~~~
{{
  :f:SATURATE1 (|f:-n) n:MAX n:to-float ;
  :f:overflow-check1 (|f:n-n)
    f:dup f:abs f:SATURATE1 f:gt? 
    [ f:put nl 'f:var1_overflow s:abort ] if ;
---reveal---
  :f:var1   (s-|f:n-) f:overflow-check1 f:to-w1 rot d:create , , ;
  :f:fetch1 (a-|f:-n) fetch-next [ fetch ] dip w1:to-f ;
  :f:store1 (a-|f:n-) f:overflow-check1 f:to-w1 rot store-next store ;
}}
~~~

```
.-123456789.0987654321 'FVar1 f:var1
&FVar1 f:fetch1 st
.-98765.4321 &FVar1 f:store1
&FVar1 f:fetch1 st
```

  .-9876543210.123456789 &FVar1 f:store1 (This_test_should__abort
  .1.e20 &FVar1 f:store1                 (This_test_should__abort

~~~
{{
  :f:SATURATE2 (|f:-n) n:MAX n:to-float f:dup f:* ;
  :f:overflow-check2 (|f:n-n)
    f:dup f:abs f:SATURATE2 f:gt? 
    [ f:put nl 'f:var2_overflow s:abort ] if ;
---reveal---
  :f:var2   (s-|f:n-) f:overflow-check2 f:to-w2 rot d:create , , ;
  :f:fetch2 (a-|f:-n) fetch-next [ fetch ] dip w2:to-f ;
  :f:store2 (a-|f:n-) f:overflow-check2 f:to-w2 rot store-next store ;
}}
~~~

```
.-123456789.0987654321 'FVar2 f:var2
&FVar2 f:fetch2 st
.-9876543210.123456789 &FVar2 f:store2
&FVar2 f:fetch2 st
```

  .1.e20 &FVar2 f:store2  (This_test_should_abort
