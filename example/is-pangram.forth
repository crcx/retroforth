~~~
{{
  'abcdefghijklmnopqrstuvwxyz 'FULL s:const
  '__________________________ 'TEST s:const
---reveal---
  :s:pangram? (s-f)
    '__________________________ &TEST #26 copy
    s:to-lower [ c:letter? ] s:filter [ dup $a - &TEST + store ] s:for-each
    &TEST &FULL s:eq? ;
}}
~~~

'Hello_world! s:pangram?
'The_quick_brown_fox_jumped_over_the_lazy_dogs. s:pangram?
