The `buffer:` words are one of the few direct holdovers
from RETRO11. I use them frequently, but don't have a
good single example or description. So let's rectify
that now.

A buffer is just a linear sequence of memory. These
words provide a way to incrementally store or retrieve
values from it.

To begin, create a memory region to use as a buffer.

~~~
'Test d:create #1025 allot
~~~

Then you can set this as the current buffer:

~~~
Test buffer:set
~~~

When a buffer is set, the vocabulary sets an internal
index to the first address in it. This will be
incremented when you add data and decremented when you
remove data.

Let's add some stuff using `buffer:add`:

~~~
#100 buffer:add
#200 buffer:add
#300 buffer:add
~~~

And then retreive the values:

~~~
buffer:get n:put nl
buffer:get n:put nl
buffer:get n:put nl
~~~

You can remove all values using `buffer:empty`:

~~~
#100 buffer:add
#200 buffer:add
#300 buffer:add
buffer:empty
~~~

And ask the buffer how many items it contains:

~~~
buffer:size n:put nl
#100 buffer:add
#200 buffer:add
#300 buffer:add
buffer:size n:put nl
buffer:empty
~~~

The other functions are `buffer:start`, which returns
the address of the buffer, `buffer:end`, which returns
the address of the last value, and `buffer:preserve`.
The first is easy to demo:

~~~
buffer:start Test eq? n:put nl
~~~

The last one is useful. Only one buffer is ever active
at a given time. The `buffer:preserve` combinator lets
you execute a word, saving and restoring the current
buffer indexes. So the word could assign and use a new
buffer and this will reset the previous one after
control returns.

There are a few notes that need to be considered. The
preserve combinator saves the start and current index
but *not* the contents. If the word you call uses the
same buffer, the contents will remain altered.

Finally, the buffer words have one interesting trait:
they store an ASCII NULL after adding each item to the
buffer. This lets one use them to build strings easily.

~~~
Test buffer:set
$h buffer:add
$e buffer:add
$l buffer:add
$l buffer:add
$o buffer:add
$, buffer:add
#32 buffer:add
$w buffer:add
$o buffer:add
$r buffer:add
$l buffer:add
$d buffer:add
buffer:start s:put nl
~~~
