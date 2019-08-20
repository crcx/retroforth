# Sorting Numbers on the Stack

This is a recursive approach to sorting values on the stack. I
won't try to claim that this is efficient, but it works.

~~~
:sort-pair    dup-pair lt? &swap if ;
:perform-sort sort-pair depth #2 gt? [ &perform-sort dip ] if ;
:sort         depth &perform-sort times ;
~~~

```
#3 #33 #22 #333 #5 sort
```
