# Overview

RETRO has a significant number of differences from traditional
Forth systems. This will provide a number of aliases as well as
words from traditional Forth systems.

It is not possible to provide a traditional Forth as a superset
of RETRO due to the way the parsing and token processing works.
Implementing a traditional Forth using RETRO as the implementation
language would be possible (within the limits of the Nga virtual
machine), but I have no desire to do this.

Where letters are used, I am using uppercase. This is in keeping
with tradition as well as the minimal requirements of ANS.

## Memory Access

~~~
:@  (a-n)  fetch ;
:!  (na-)  store ;
:C@ (a-n)  fetch ;
:C! (na-)  store ;
:@+ (a-an) fetch-next ;
:!+ (na-a) store-next ;
~~~

## Return Stack

~~~
:>R (n-)   |push ; immediate
:R> (-n)   |pop ; immediate
:R@ (-n)   |pop |dup |push ; immediate
~~~

## Conditionals

~~~
:IF   FALSE |[ ; immediate
:ELSE |] drop TRUE |[ ; immediate
:THEN |] [ |choose ] [ |if ] choose ; immediate
:=    eq? ;
:<>   -eq? ;
:<    lt? ;
:>    gt? ;
:<=   lteq? ;
:>=   gteq? ;
~~~

## Loops

~~~
:BEGIN |repeat ; immediate
:AGAIN |again ; immediate
:FOR   |[ ; immediate
:NEXT  |] |times<with-index> ; immediate
~~~

## I/O

~~~
:KEY    (-c)  c:get ;
:EMIT   (c-)  c:put ;
:.      (n-)  n:put sp ;
:TYPE   (an-) &c:put times ;
:SPACE  (-)   sp ;
:SPACES (n-)  &sp times ;
~~~

## Maths

~~~
:/MOD /mod ;
:0<   n:negative? ;
:0=   n:zero? ;
:0>   n:positive? ;
:1+   n:inc ;
:1-   n:dec ;
:MAX  n:max ;
:MIN  n:min ;
:MOD  mod ;
:NEGATE n:negate ;
:AND  and ;
:OR   or ;
:XOR  xor ;
~~~

## Stack

~~~
:DUP  dup ;
:DROP drop ;
:SWAP swap ;
:OVER over ;
:TUCK tuck ;
:ROT  rot ;
:-ROT rot rot ;

{{
  :save-stack (...-a)
    here [ depth &, times ] dip ;
  :fetch-prior (a-n[a-1])
    dup fetch swap n:dec ;
  :restore-stack (a-...)
    here swap - here n:dec swap [ fetch-prior ] times drop ;
---reveal---
  :PICK (...n-...m)
    &Heap [ [ save-stack ] dip
            over + fetch [ restore-stack ] dip ] v:preserve ;
}}

{{
  :save-values (...n-a)
    [ , ] times here ;
  :restore-values (a-...)
    here - here swap [ fetch-next swap ] times drop ;
---reveal---
  :ROLL (...n-...m)
    &Heap [ save-values ] v:preserve swap [ restore-values ] dip ;
}}
~~~

## Strings

Traditional Forth systems use a variety of strings. I define words
to translate RETRO strings into these.

~~~
:caddr/u  (a-an) dup s:length ;
:counted  (a-a)  here [ dup s:length , s, ] dip ;
:unpack   (a-a)  fetch-next here [ [ fetch-next , ] times drop #0 , ] dip ;
:COUNT    (a-an) fetch-next ;
~~~

## Compiler

~~~
:IMMEDIATE immediate ;
:STATE     &Compiler ;
:LITERAL   compile:lit ; immediate
~~~

## Dictionary

~~~
:FIND (caddr-caddr,0|xt,1|xt,-1)
  dup unpack d:lookup dup
  n:zero? [ drop #0 ] if; nip
  [ d:xt fetch ]
  [ d:class fetch &class:macro eq? [ #1 ] [ #-1 ] choose ] bi ;
~~~

## Unsorted

~~~
:CHAR+  n:inc ;
:CHARS  ;
:CELL+  n:inc ;
:CELLS  ;
:BL     ASCII:SPACE ;
~~~

## Unsupported

ANS, FIG, and other models have many words that I don't use or
can't support directly in RETRO without changing the design of
my system. The lists below are not exhaustive, but should be
helpful in identifying things that may make porting code more
difficult.

The following words are not supported due to design choices in RETRO.

    --------  --------  --------  --------  --------  --------
    #         #>        #S        '         (         ."
    [']       :         >IN       [CHAR]    ABORT"    CONSTANT
    CREATE    POSTPONE  S"        SOURCE    VARIABLE  WORD
    --------  --------  --------  --------  --------  --------

The following are not supported due to limitations of the VM.

    --------  --------  --------  --------  --------  --------
    U.        U<        UM*       UM/MOD
    --------  --------  --------  --------  --------  --------

The following I do not plan to implement.

    --------  --------  --------  --------  --------  --------
    <#        >BODY     ABORT     BASE      ENVIRONMENT?
    FM/MOD    QUIT      SM/REM    [         ]
    --------  --------  --------  --------  --------  --------
