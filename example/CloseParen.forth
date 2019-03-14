I find `)` useful to punctuate code chunks.

~~~
:) ; immediate (code_chunk_punctuation_that_does_nothing
~~~

Here is an example:

  :i@ (an-n) array:nth fetch ;

~~~
{{
  :I@ (an-n) I array:nth fetch ;
  'Index 'Value [ var ] bi@
  :lt?-or-gt? hook ;
  :pre.min (a-an)
    (comparison  &lt? &lt?-or-gt? set-hook       )
    (begin_with  #-1 !Index n:MAX !Value dup array:length ) ;
  :pre.max (a-an)
    (comparison  &gt? &lt?-or-gt? set-hook       )
    (begin_with  #-1 !Index n:MIN !Value dup array:length ) ;
  :min-or-max (a-nn) [ dup I@ dup @Value lt?-or-gt?
    [ !Value I !Index ] [ drop ] choose ] times<with-index>
    (a-nn drop @Index @Value ) ;
---reveal---
 :array:min (a-iv) pre.min min-or-max ;
 :array:max (a-iv) pre.max min-or-max ;
}}
~~~

```
{ #3 #2 #5 #7 #3 } array:min dump-stack nl reset
{ #3 #2 #5 #7 #3 } array:max dump-stack nl
```

