# Save and Restore Stack

It's sometimes useful to temporarily save and restore the entire
stack. These two words allow for this.

## The Code

~~~
:stack:save    (-a)
  here [ depth dup , &, times ] dip ;

:stack:restore (a-)
  &reset dip
  dup fetch over + swap fetch [ dup fetch swap n:dec ] times drop ;
~~~

## Test Case

```
#1 #2 #3 #4 #5
stack:save #3 swap stack:restore
```
