Short for *top of return stack*, this returns the top item on the
address stack. As an analog to traditional Forth, this is equivilent
to `R@`.

~~~
:tors (-n)  pop pop dup push swap push ;
~~~
