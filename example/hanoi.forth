~~~
'Num  var
'From var
'To   var
'Via  var

:set-vars !Via !To !From !Num ;
:hanoi (num,from,to,via-)
  set-vars
  @Num n:-zero?
  [
    @Num @From @To @Via
    @Num n:dec @From @Via @To hanoi
    set-vars
    @To @From '\nMove_a_ring_from_%n_to_%n s:with-format puts
    @Num n:dec @Via @To @From hanoi
  ] if ;

#3 #1 #3 #2 hanoi nl
~~~
