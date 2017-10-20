Traditional Forth has a CREATE/DOES> construct. RETRO allows for
something similar using the `does` combinator.

An example in traditional Forth:

    : acc ( n "name" -- )
      create , does> dup >r @ dup 1+ r> ! ;

And in RETRO, using `does` and the `bi` combinator:

~~~
:acc (ns-)
  d:create , [ [ fetch ] [ v:inc ] bi ] does ;
~~~

Here's a little test case:

~~~
#10 'foo acc
foo
foo
foo
~~~
