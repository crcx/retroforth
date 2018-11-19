#!/usr/bin/env retro

~~~
{{
  'Fenced var
  :toggle-fence @Fenced not !Fenced ;
  :fenced? (-f) @Fenced ;
  :handle-line (s-)
    fenced? [ over call ] [ drop ] choose ;
---reveal---
  :unu (sq-)
    swap [ dup '~~~ s:eq?
           [ drop toggle-fence ]
           [ handle-line       ] choose
         ] file:for-each-line drop ;
}}
~~~

~~~
#0 sys:argv [ s:put nl ] unu
~~~

