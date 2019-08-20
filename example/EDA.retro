# EDA.forth

## Description

Forth EDA, ported to RETRO
http://www.0xff.in/bin/A_Language_for_Digital_Design.pdf

## Code & Commentary

~~~
#32768 'STATUS const
#32767 'TIME const
~~~

### Digital logic simulator

~~~
:S. dup STATUS and n:-zero? [ '+ s:put ] if TIME and n:put ;
:_ (n-n) STATUS xor ;
:eda:and (nn-n) over  TIME and over  TIME and n:max rot  STATUS and rot  STATUS and and + ;
:eda:or (nn-n) _ swap _ eda:and _ ;
~~~

### Technology

~~~
:2and (nn-n)  eda:and _ #9 + ;
:3or  (nnn-n) eda:or eda:or #30 + ;
:2xor (nn-n)  over _ over eda:and [ _ eda:and ] dip eda:or #35 + ;
~~~

### Logic equations

~~~
#0    'A const
#10   'B const
#10 _ 'C const
:enb_ (-n) A B 2xor ; (45)
:xy   (-n) enb_ C 2and A B 3or ; (+84)
~~~

## Test

```
enb_ S. nl
xy S. nl
```
