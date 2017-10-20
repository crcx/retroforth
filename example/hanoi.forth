~~~
'a var
'b var
'c var
'n var

:vars !c !b !a !n ;
:hanoi (num,from,to,via-)
  vars
  @n n:-zero?
  [
    @n @a @b @c
    @n n:dec @a @c @b hanoi
    vars
    @b @a '\nMove_a_ring_from_%n_to_%n s:with-format puts
    @n n:dec @c @b @a hanoi
  ] if ;

#5 #1 #3 #2 hanoi nl
~~~
