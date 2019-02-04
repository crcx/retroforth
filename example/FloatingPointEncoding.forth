# Float

## Description

This implements a means of encoding floating point values into signed integer cells. The technique is described in the paper titled "Encoding floating point numbers to shorter integers" by Kiyoshi Yoneda and Charles Childers.

This will extend the `f:` vocabulary and adds a new `u:` vocabulary.

## Code & Commentary

Define some constants. The range is slightly reduced from the standard integer range as the smallest value is used for NaN.

~~~
n:MAX n:dec          'u:MAX const
n:MAX n:dec n:negate 'u:MIN const
n:MIN                'u:NAN const
n:MAX                'u:INF const
n:MAX n:negate       'u:-INF const
~~~

~~~
:u:n?    (u-f)
  u:MIN n:inc u:MAX n:dec n:between? ;
:u:max?  (u-f) u:MAX eq? ;
:u:min?  (u-f) u:MIN eq? ;
:u:zero? (u-f) n:zero? ;
:u:nan?  (u-f) u:NAN eq? ;
:u:inf?  (u-f) u:INF eq? ;
:u:-inf? (u-f) u:-INF eq? ;
:u:clip  (u-u) u:MIN u:MAX n:limit ;
~~~

~~~
{{
  :s .10.0e-4 ;
  :f:encode .0.5 f:power s .-1.0 f:power f:* ;
  :f:decode f:square s f:square f:* ;

---reveal---

  :f:to-u  (-u|f:a-)
    f:dup f:encode f:round f:to-number u:clip
    f:dup f:nan? [ drop u:NAN ] if
    f:dup f:inf? [ drop u:INF ] if
    f:dup f:-inf? [ drop u:-INF ] if
    f:drop ;

  :u:to-f  (u-|f:-b)
    dup n:to-float f:decode
    dup u:nan?  [ f:drop f:NAN ] if
    dup u:inf?  [ f:drop f:INF ] if
    dup u:-inf? [ f:drop f:-INF ] if
    drop ;
}}
~~~

```
:tab #9 c:put #9 c:put #9 c:put ;
.103.333 f:to-u dup n:put tab u:to-f f:put nl
..000412 f:to-u dup n:put sp sp tab u:to-f f:put nl
.129192.232415 f:to-u dup n:put tab u:to-f f:put nl
f:NAN f:to-u dup n:put sp sp sp sp sp u:to-f f:put nl
f:INF f:to-u dup n:put sp sp sp sp sp u:to-f f:put nl
```

