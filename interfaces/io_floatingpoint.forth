~~~
{{
  'io:FloatingPoint var
  :identify
    @io:FloatingPoint n:zero? [
      #2 io:scan-for dup n:negative?
      [ drop 'IO_DEVICE_TYPE_0002_NOT_FOUND s:put nl ]
      [ !io:FloatingPoint ] choose ] if ;
  ---reveal---
  :io:float-operation identify @io:FloatingPoint io:invoke ;
}}
~~~

# Floating Point

~~~
:n:to-float  (n-_f:-n)   #0 io:float-operation ;
:s:to-float  (s-_f:-n)   #1 io:float-operation ;
:f:to-number (f:a-__-n)  #2 io:float-operation ;
:f:to-string (f:n-__-s) s:empty dup #3 io:float-operation ;
:f:+     (f:ab-c)    #4 io:float-operation ;
:f:-     (f:ab-c)    #5 io:float-operation ;
:f:*     (f:ab-c)    #6 io:float-operation ;
:f:/     (f:ab-c)    #7 io:float-operation ;
:f:floor (f:ab-c)    #8 io:float-operation ;
:f:ceiling (f:f-f)   #9 io:float-operation ;
:f:sqrt  (f:f-f)    #10 io:float-operation ;
:f:eq?   (f:ab-c)   #11 io:float-operation ;
:f:-eq?  (f:ab-c)   #12 io:float-operation ;
:f:lt?   (f:ab-c)   #13 io:float-operation ;
:f:gt?   (f:ab-c)   #14 io:float-operation ;
:f:depth (-n)       #15 io:float-operation ;
:f:dup   (f:a-aa)   #16 io:float-operation ;
:f:drop  (f:a-)     #17 io:float-operation ;
:f:swap  (f:ab-ba)  #18 io:float-operation ;
:f:log   (f:ab-c)   #19 io:float-operation ;
:f:power (f:ab-c)   #20 io:float-operation ;
:f:sin   (f:f-f)    #21 io:float-operation ;
:f:cos   (f:f-f)    #22 io:float-operation ;
:f:tan   (f:f-f)    #23 io:float-operation ;
:f:asin  (f:f-f)    #24 io:float-operation ;
:f:acos  (f:f-f)    #25 io:float-operation ;
:f:atan  (f:f-f)    #26 io:float-operation ;
:f:square (f:n-m)   f:dup f:* ;
:f:over  (f:ab-aba) f:to-string f:dup s:to-float f:swap ;
:f:tuck  (f:ab-bab) f:swap f:over ;
:f:positive? (-f__f:a-) #0 n:to-float f:gt? ;
:f:negative? (-f__f:a-) #0 n:to-float f:lt? ;
:f:negate (f:a-b)  #-1 n:to-float f:* ;
:f:abs    (f:a-b)  f:dup f:negative? [ f:negate ] if ;
:prefix:. (s-__f:-a)
  compiling? [ s:keep ] [ s:temp ] choose &s:to-float class:word ; immediate
:f:put (f:a-) f:to-string s:put ;
:f:PI (f:-F) .3.141592 ;
:f:E  (f:-F) .2.718281 ;
:f:NAN (f:-n) .0 .0 f:/ ;
:f:INF (f:-n) .1.0 .0 f:/ ;
:f:-INF (f:-n) .-1.0 .0 f:/ ;
:f:nan? (f:n-,-f) f:dup f:-eq? ;
:f:inf? (f:n-,-f) f:INF f:eq? ;
:f:-inf? (f:n-,-f) f:-INF f:eq? ;
:f:round (-|f:a-b)
  f:dup f:negative?
  [ .0.5 f:- f:ceiling ]
  [ .0.5 f:+ f:floor   ] choose ;
~~~

---------------------------------------------------------------

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
    n:to-float f:decode ;

  :u:to-f  (u-|f:-b)
    dup n:to-float f:decode
    dup u:nan?  [ f:drop f:NAN ] if
    dup u:inf?  [ f:drop f:INF ] if
    dup u:-inf? [ f:drop f:-INF ] if
    drop ;
}}
~~~

~~~
:f:store (a-|f:n-) [ f:to-u ] dip store ;
:f:fetch (a-|f:-n) fetch u:to-f ;
~~~