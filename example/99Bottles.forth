# 99 Bottles

Display the text for the *99 Bottles of Beer* song.

~~~
[ dup n:put sp 'bottles s:put ]
[ '1_bottle s:put ]
[ 'no_more_bottles s:put  ]
'bottles d:create
  , , ,

:display-bottles
  dup #2 n:min bottles + fetch call ;

:display-beer
  display-bottles '_of_beer s:put ;

:display-wall
  display-beer '_on_the_wall s:put ;

:display-take
  'Take_one_down,_pass_it_around  s:put ;

:display-verse
  display-wall nl display-beer nl
  n:dec display-take nl display-wall nl ;

:?dup
  dup 0; ;

:verses
  [ nl display-verse dup n:-zero? ] while drop ;
 
#99 verses
~~~
