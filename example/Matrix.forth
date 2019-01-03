#!/usr/bin/env retro -i -s

By WilhelmVonWeiner: Matrices that store their bounds and a
couple rushedly written tests.

~~~
{{
  :prepare (nms--knms) push dup-pair * rot rot pop ;
  :create (nms--) d:create dup-pair * #2 + allot ;
  :initialise (nm--) d:last<xt> store-next store-next ;
  :fill (na--a)  [ store-next ] times ;
---reveal---
  :matrix (nms--a) create initialise ;
  :matrix<xs> (xn...x1nms-) prepare matrix swap fill drop ;
}}
~~~

Test matrix, should print "matrix works!" and not "broken!".

```
:get-next n:dec dup fetch ;
:broken 'broken s:put nl ;
#3 #1 'tester matrix
get-next #3 -eq? [ broken ] if
get-next #1 -eq? [ broken ] if
'matrix_works!_at_ s:put n:put nl
```

Test matrix<xs>, should be "contained!" thrice.

```
#30 #20 #10 #3 #1 "tester matrix<xs>
{ tester #2 + #3 [ fetch-next swap ] times drop }
[ { #10 #20 #30 } set:contains? [ 'contained! s:put sp nl ] if ] set:for-each
```
