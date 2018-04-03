LISP provides a function, `defstruct`, which creates a data
structure and functions for accessing various fields in it.
This can be useful, so I'm doing something similar here.

(defstruct book title author subject book-id )

~~~
{{
  :make-helper (nsq-)  [ d:create , ] dip does ;
  :make-struct (ns-)   d:create , [ here swap fetch allot ] does ;
---reveal---
  :defstruct (sq-)
    set:from-results dup set:length
    [ n:dec swap
      [ 'ab 'aabab reorder
        '@ s:append [ fetch + fetch ] make-helper
        '! s:append [ fetch + store ] make-helper
        n:dec
      ] set:for-each drop
    ] sip swap make-struct ;
}}
~~~


~~~
'book [ 'title 'author 'subject 'book-id ] defstruct

book 'A const
'The_Hobbit     s:keep &A title!
'J.R.R._Tolkien s:keep &A author!
'Fantasy        s:keep &A subject!

:info (a-)
  [ subject@ ] [ author@ ] [ title@ ] tri
  '%s_by_%s_is_a_%s_book. s:with-format puts nl ;

&A info
~~~
