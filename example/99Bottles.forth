# 99 Bottles

Display the text for the *99 Bottles of Beer* song.

~~~
{ 'bottle 'bottles 'of 'beer 'on 'the 'wall 'no 'more
  'Take 'one 'down, 'pass 'it 'around }
[ dup ':%s_'%s_s:put_sp_; s:format s:evaluate ] a:for-each

{ [ no more bottles      ]
  [ #1 n:put sp bottle   ]
  [ dup n:put sp bottles ]
} 'BOTTLES const

:number-bottles
  dup #2 n:min BOTTLES swap a:fetch call ;

:display-verse
  number-bottles of beer on the wall  nl
  number-bottles of beer              nl
  n:dec Take one down, pass it around nl
  number-bottles of beer on the wall  nl ;

:verses (n-)
  repeat 0; nl display-verse again ;

#99 verses 
~~~
