~~~
0x60 'keyboard:DATA  const
0x64 'keyboard:STATUS const

:keyboard:wait
  [ keyboard:STATUS pio:in-byte #1 and n:-zero? ] until ;

:keyboard:read
  keyboard:DATA pio:in-byte ;

:uuu
  keyboard:wait
  #0 [ drop keyboard:read 0x7F and dup 0x39 gt? [ drop keyboard:wait #0 ] if dup #0 -eq? ] until ;

'keyboard:Dvorak d:create
  #0 , #27 , $1 , $2 , $3 , $4 , $5 , $6 , $7 , $8 , $9 , $0 , $[ , $] , #8 ,
  #9 ,  $' , $, , $. , $p , $y , $f , $g , $c , $r , $l , $/ , $= , #10 ,
  #0 ,  $a , $o , $e , $u , $i , $d , $h , $t , $n , $s ,  $- ,  $` ,
  #-1 ,  $\ , $; , $q , $j , $k , $x , $b , $m , $w , $v , $z ,
  #-1 ,  $* , #0 , #32 , #32 ,

'keyboard:Dvorak:Shifted d:create
  #0 , #27 , $! , $@ , $# , $$ , $% , $^ , $& , $* , $( , $) , ${ , $} , #8 ,
  #9 ,  $" , $< , $> , $P , $Y , $F , $G , $C , $R , $L , $? , $+ , #10 ,
  #0 ,  $A , $O , $E , $U , $I , $D , $H , $T , $N , $S ,  $_ ,  $~ ,
  #-1 ,  $| , $: , $Q , $J , $K , $X , $B , $M , $W , $V , $Z ,
  #-1 ,  $* , #0 , #32 , #32 ,

'Shift var

:n &keyboard:Dvorak + fetch ;
:N &keyboard:Dvorak:Shifted + fetch ;

:getc uuu drop uuu n dup #-1 eq? [ drop uuu drop uuu N ] if dup c:put ;
:test #10 &getc times reset nl ;
~~~

