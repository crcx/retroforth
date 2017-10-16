~~~
:n:to-float  (n-_f:-n)   #0 `-6000 ;
:s:to-float  (s-_f:-n)   #1 `-6000 ;
:f:to-string (f:n-__-s) s:empty dup #2 `-6000 ;
:f:+     (f:ab-c)    #3  `-6000 ;
:f:-     (f:ab-c)    #4 `-6000 ;
:f:*     (f:ab-c)    #5 `-6000 ;
:f:/     (f:ab-c)    #6 `-6000 ;
:f:floor (f:ab-c)    #7 `-6000 ;
:f:eq?   (f:ab-c)    #8 `-6000 ;
:f:-eq?  (f:ab-c)    #9 `-6000 ;
:f:lt?   (f:ab-c)   #10 `-6000 ;
:f:gt?   (f:ab-c)   #11 `-6000 ;
:f:depth (-n)       #12 `-6000 ;
:f:dup   (f:a-aa)   #13 `-6000 ;
:f:drop  (f:a-)     #14 `-6000 ;
:f:swap  (f:ab-ba)  #15 `-6000 ;
:f:over  (f:ab-aba) f:to-string f:dup s:to-float f:swap ;
:f:tuck  (f:ab-bab) f:swap f:over ;
:f:positive? #0 n:to-float f:gt? ;
:f:negative? #0 n:to-float f:lt? ;
:f:negate #-1 n:to-float f:* ;
:f:abs    f:dup f:negative? [ f:negate ] if ;
:f:log   (f:ab-c)  #16 `-6000 ;
:f:power   (f:ab-c)  #17 `-6000 ;
:f:to-number (f:a-__-n)  #18 `-6000 ;
:prefix:. (s-__f:-f)
  compiling? [ s:keep ] [ s:temp ] choose &s:to-float class:word ; immediate
:putf (f:-) f:to-string puts ;
~~~

