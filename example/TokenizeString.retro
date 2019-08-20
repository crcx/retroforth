If you want to tokenize a string into a set, this is one approach.

~~~
{{
  'Split-On var
  :match?    (c-f) @Split-On eq? ;
  :terminate (s-s) #0 over n:dec store ;
  :step      (ss-s) [ n:inc ] dip match? [ dup , terminate ] if ;
---reveal---
  :s:tokenize (sc-a)
    !Split-On s:keep
    here #0 , [ dup , dup [ step ] s:for-each drop ] dip
    here over - n:dec over store ;
}}
~~~

