#!/usr/bin/env rre

This is a simple `echo` style example.

~~~
#0 sys:argc
~~~

Then a simple loop:

- duplicate the argument number
- get the argument as a string
- display it, followed by a space
- increment the argument number

~~~
[ dup sys:argv s:put sp n:inc ] times
~~~

And at the end, discard the argument number and inject a
newline.

~~~
drop nl
~~~
