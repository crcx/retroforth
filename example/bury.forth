This is a word to bury a value by moving it to the bottom
of the stack.

It does this in a quick and dirty way: copy the values other
than TOS into a new array, then copy the values from the
array back to the stack. This is slow, but it's not something
that I've ever needed in actual use, so I see no reason to
devote time to finding a faster solution.

~~~
:bury (...n-n...)
  &Heap [ here [ [ depth dup , [ , ] times ] dip ] dip
          a:reverse [ ] a:for-each ] v:preserve ;
~~~

Test Case:

```
#12 #23 #34 #45 #56
dump-stack nl
bury
nl dump-stack nl
```
