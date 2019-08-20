This is an example adding two three element vectors.

~~~
:vadd (v1v2v3-)
  'abc  'cabcabcab reorder
  [ #2 +  ] tri@ [ fetch ] bi@ + swap store
  [ n:inc ] tri@ [ fetch ] bi@ + swap store
                 [ fetch ] bi@ + swap store ;
~~~

A test case:

```
'a d:create #1 , #2 , #3 ,
'b d:create #2 , #3 , #4 ,
'c d:create #3 allot

&a &b &c vadd

&c fetch-next n:put nl
   fetch-next n:put nl
   fetch      n:put nl
```

