~~~
:MAP    (-s)  'nopqrstuvwxyzabcdefghijklm ;
:encode (c-c) $a - MAP + fetch ;
:rot13  (s-s) s:to-lower [ dup c:letter? [ encode ] if ] s:map ;
~~~

