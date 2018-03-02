The standard RETRO language provides `bi` and `tri` combinators to apply quotes to two or three values in various combinations. Sometimes it may be necessary to do this with four values.

Note that this is *ugly* code. It's functional, but if you can refactor to avoid needing it, it'll likely be better in the long run.

`quad` applies four quotes to a value. These are equivilent:

  #10 [ #1 + ] call
  #10 [ #1 - ] call
  #10 [ #2 + ] call
  #10 [ #3 + ] call

  #10 [ #1 + ] [ #1 - ] [ #2 + ] [ #3 + ] quad

~~~
:quad (xqqqq-)
  'abcde 'abacadae reorder
  push push push push push push
  call pop pop call pop pop call
  pop pop call ;
~~~

`quad*` takes eight values (!) and applies each quote to a specific value. E.g., these are equivilent:

  #10 [ #1 + ] call
  #11 [ #1 - ] call
  #12 [ #2 + ] call
  #13 [ #3 + ] call

  #10 #11 #12 #13 [ #1 + ] [ #1 - ] [ #2 + ] [ #3 + ] quad*

~~~
:quad* (abcdqqqq-)
  'abcdefgh 'aebfcgdh reorder
  push push push push push push
  call pop pop call pop pop call
  pop pop call ;
~~~

`quad@` takes four values and a quote, and applies the quote to each value in order. These are equivilent:

  #10 [ #1 + ] call
  #11 [ #1 + ] call
  #12 [ #1 + ] call
  #13 [ #1 + ] call

  #10 #11 #12 #13 [ #1 + ] quad@

~~~
:quad@ (abcdq-)
  'abcde 'abcdeeee reorder quad* ;
~~~

