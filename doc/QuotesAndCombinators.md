# Quotes and Combinators

RETRO makes extensive use of two elements that need some explanation. These
are called quotes and combinators.

A *quote* is an anonymous function. They are nestable and can be created at
any time.

Example:

~~~
  #12 [ dup * ] call
~~~

In this, the code stating with `[` and ending with `]` is the quote. Here
it's just `call`ed immediately, but you can also pass it to other words.

We use the word *combinator* to refer to words that operate on quotes.

You'll use quotes and combinators extensively for controlling the flow of
execution. This begins with conditionals.

Assuming that we have a flag on the stack, you can run a quote if the
flag is `TRUE`:

~~~
  #1 #2 eq?
  [ 'True! s:put ] if
~~~

Or if it's `FALSE`:

~~~
  #1 #2 eq?
  [ 'Not_true! s:put ] -if
~~~

There's also a `choose` combinator:

~~~
  #1 #2 eq?
  [ 'True! s:put     ]
  [ 'Not_true! s:put ] choose
~~~

RETRO also uses combinators for loops:

A counted loop takes a count and a quote:

~~~
  #0 #100 [ dup n:put sp n:inc ] times
~~~

You can also loop while a quote returns a flag of `TRUE`:

~~~
  #0 [ n:inc dup #100 lt? ] while
~~~

Or while it returns `FALSE`:

~~~
  #100 [ n:dec dup n:zero? ] until
~~~

In RETRO you can also use combinators to iterate over data structures. For
instance, many structures provide a `for-each` combinator which can be run
once for each item in the structure. E.g., with a string:

~~~
  'Hello [ c:put ] s:for-each
~~~

Moving further, combinators are also used for filters and operations on
data. Again with strings:

~~~
  'Hello_World! [ c:-vowel? ] s:filter
~~~

This runs `s:filter` which takes a quote returning a flag. For each `TRUE`
it appends the character into a new string, while `FALSE` results are
discarded.

You might also use a `map`ping combinator to update a data set:

~~~
  'Hello_World [ c:to-upper ] s:map
~~~

This takes a quote that modifies a value which is then used to build a
new string.

There are many more combinators. Look in the Glossary to find them. Some
notable ones include:

    bi
    bi*
    bi@
    tri
    tri*
    tri@
    dip
    sip
