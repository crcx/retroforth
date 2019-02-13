Words to encode floating point numbers into two cells. 

~~~
:st (-) dump-stack nl f:dump-stack ; 
  :f:sign (-n|f:a-) 
  f:dup .0 f:eq? [ #0 f:drop ] if; 
  .0 f:gt? [ #1 ] [ #-1 ] choose ; 
~~~

~~~
:f:encode.w (|f:n-n) f:dup f:sign f:abs f:sqrt n:to-float f:* ; 
:f:decode.w (|f:n-n) f:dup f:sign f:square n:to-float f:* ; 
~~~

```
.-12345.6789 f:encode.w st 
f:decode.w st 
```

~~~
{{ 
  :f:-shift9 .1.e9 f:* ; 
  :f:shift9 .1.e-9 f:* ; 
---reveal--- 
  :f:split (-|f:n-fi)_absFrac.-shift9_signedInt 
    f:dup f:sign (s|f:_n 
    f:abs f:dup f:floor f:swap f:over (s|f:_abs.int_abs_abs.int 
    f:- f:-shift9 (s|f:_abs.int_abs.frac.-shift9 
    f:swap n:to-float f:* (_|f:_abs.frac.-shift9_signedInt 
  ; 
  :f:join (-|f:fi-n) 
    f:dup f:sign f:abs (s|f:_abs.frac.-shift9_abs.int 
    f:swap f:shift9 f:+ n:to-float f:* (_|f:_n 
  ; 
}} 
~~~

```
.-123456789.0987654321 f:split st 
f:join st 
```

~~~
:f:to-w (-fi|f:n-)_frac_sInt 
  f:encode.w f:split f:to-number f:to-number swap ; 

:w:to-f (fi-|f:-n) 
  swap n:to-float n:to-float (_|f:_abs.frac_signedInt 
  f:join f:decode.w ; 
~~~

```
.-123456789.0987654321 f:to-w st 
w:to-f st 
```

~~~
:f:var (s-|f:n-) f:to-w rot d:create , , ; immediate 
:f:fetch (a-|f:-n) fetch-next [ fetch ] dip w:to-f ; 
:f:store (a-|f:n-) f:to-w rot store-next store ; 
~~~

```
.-123456789.0987654321 'FVar f:var 
&FVar f:fetch st 
.-9876543210.123456789 &FVar f:store 
&FVar f:fetch st 
```
