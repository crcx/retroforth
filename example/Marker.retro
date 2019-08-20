Marker provides a way to quickly reset the dictionary and heap
to the state it was in prior to the creation of the marker.

## The Code

~~~
:class:marker (a-)
  compiling? [ compile:lit &class:marker compile:call ]
             [ fetch-next !Dictionary fetch !Heap     ] choose ;
:marker (s-) [ @Heap @Dictionary ] dip d:create , , &class:marker reclass ;
~~~

## A Test Case

```
:a #1 #2 #3 ;
:b a + + ;
:c b n:put nl ;
c
'd marker
:a #4 #5 #6 ;
:b a + + ;
:c b n:put nl ;
c
d
c
```
