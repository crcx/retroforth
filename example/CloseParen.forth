I find `)` useful to punctuate code chunks.

~~~
:) ; immediate (code_chunk_punctuation_that_does_nothing
~~~

Here is an example:

  :i@ (an-n) a:fetch ;

~~~
{{
  :I@ (an-n) I a:fetch ;
  'Index 'Value [ var ] bi@
  :lt?-or-gt? hook ;
  :pre.min (a-an)
    (comparison  &lt? &lt?-or-gt? set-hook       )
    (begin_with  #-1 !Index n:MAX !Value dup a:length ) ;
  :pre.max (a-an)
    (comparison  &gt? &lt?-or-gt? set-hook       )
    (begin_with  #-1 !Index n:MIN !Value dup a:length ) ;
  :min-or-max (a-nn) [ dup I@ dup @Value lt?-or-gt?
    [ !Value I !Index ] [ drop ] choose ] times<with-index>
    (a-nn drop @Index @Value ) ;
---reveal---
 :a:min (a-iv) pre.min min-or-max ;
 :a:max (a-iv) pre.max min-or-max ;
}}
~~~

```
{ #3 #2 #5 #7 #3 } a:min dump-stack nl reset
{ #3 #2 #5 #7 #3 } a:max dump-stack nl
```

