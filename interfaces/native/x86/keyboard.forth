~~~
0x60 'keyboard:DATA  const
0x64 'keyboard:STATUS const

:keyboard:wait
  [ keyboard:STATUS pio:in-byte #1 and n:zero? ] until ;

:uuu
  [ keyboard:wait
    keyboard:DATA pio:in-byte dup #0 gt? ] until ;

:n n:put sp ;

:test uuu n uuu n uuu n uuu n nl ;
~~~

