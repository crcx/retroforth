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
:f:push  (f:f-)     #27 io:float-operation ;
:f:pop   (f:-f)     #28 io:float-operation ;
:f:adepth  (-n)     #29 io:float-operation ;
:f:square (f:n-m)   f:dup f:* ;
:f:over  (f:ab-aba) f:push f:dup f:pop f:swap ;
:f:tuck  (f:ab-bab) f:dup f:push f:swap f:pop ;
:f:nip   (f:ab-b)   f:swap f:drop ;
:f:drop-pair (f:ab-) f:drop f:drop ;
:f:dup-pair (f:ab-abab) f:over f:over ;
:f:rot  (f:abc-bca)  f:push f:swap f:pop f:swap ;
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
:f:min   (f:nn-n)  f:dup-pair f:lt? &f:drop &f:nip choose ;
:f:max   (f:nn-n)  f:dup-pair f:gt? &f:drop &f:nip choose ;
:f:limit (f:nlu-n) f:swap f:push f:min f:pop f:max ;
:f:between? (f:nlu-n) f:rot f:dup f:push f:rot f:rot f:limit f:pop f:eq? ;
:f:inc   (f:n-n)   .1 f:+ ;
:f:dec   (f:n-n)   .1 f:- ;
:f:case  (f:ff-,q-)
  f:over f:eq? [ f:drop call #-1 ] [ drop #0 ] choose 0; pop drop-pair ;
:f:sign (-n|f:a-)
  f:dup .0 f:eq? [ #0 f:drop ] if;
  .0 f:gt? [ #1 ] [ #-1 ] choose ;
~~~

---------------------------------------------------------------

# Float

## Description

This implements a means of encoding floating point values
into signed integer cells. The technique is described in
the paper titled "Encoding floating point numbers to shorter
integers" by Kiyoshi Yoneda and Charles Childers.

This will extend the `f:` vocabulary and adds a new `e:`
vocabulary.

## Code & Commentary

Define some constants. The range is slightly reduced from
the standard  integer range as the smallest value is used
for NaN.

~~~
n:MAX n:dec          'e:MAX const
n:MAX n:dec n:negate 'e:MIN const
n:MIN                'e:NAN const
n:MAX                'e:INF const
n:MAX n:negate       'e:-INF const
~~~

~~~
:e:n?    (u-f)
  e:MIN n:inc e:MAX n:dec n:between? ;
:e:max?  (u-f) e:MAX eq? ;
:e:min?  (u-f) e:MIN eq? ;
:e:zero? (u-f) n:zero? ;
:e:nan?  (u-f) e:NAN eq? ;
:e:inf?  (u-f) e:INF eq? ;
:e:-inf? (u-f) e:-INF eq? ;
:e:clip  (u-u) e:MIN e:MAX n:limit ;
~~~

~~~
:e:scaling hook .10.0e-4 ;
{{
  :f:encode
    f:dup f:sign #-1 eq?
    [ f:abs f:sqrt e:scaling .-1.0 f:power f:* f:negate ]
    [       f:sqrt e:scaling .-1.0 f:power f:* ] choose ;
  :f:decode f:square e:scaling f:square f:* ;

---reveal---

  :f:to-e  (-u|f:a-)
    f:dup f:encode f:round f:to-number e:clip
    f:dup f:nan?  [ drop e:NAN ] if
    f:dup f:inf?  [ drop e:INF ] if
    f:dup f:-inf? [ drop e:-INF ] if
    f:drop ;

  :e:to-f  (u-|f:-b)
    dup n:to-float f:decode
    dup n:negative? [ f:negate ] if
    dup e:nan?  [ f:drop f:NAN ] if
    dup e:inf?  [ f:drop f:INF ] if
    dup e:-inf? [ f:drop f:-INF ] if
    drop ;
}}
~~~

~~~
:f:store (a-|f:n-) [ f:to-e ] dip store ;
:f:fetch (a-|f:-n) fetch e:to-f ;
~~~

~~~
:f:dump-stack f:depth dup [ f:push ] times [ f:pop f:dup f:put sp ] times ;
~~~
