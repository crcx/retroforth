~~~
{{
  :random:byte 
    '/dev/urandom file:R file:open
    &file:read sip file:close ;
---reveal---
  :n:random
    random:byte   #-8 shift
    random:byte + #-8 shift
    random:byte + #8 shift
    random:byte + ;
}}
~~~


## xoroshiro128**

XOR/rotate/shift/rotate PNRG (See http://xoshiro.di.unimi.it/)

~~~
{{
  'k var
  't var 's0 var 's1 var 's2 var 's3 var
  :seed (n-) dup !k dup !t dup !s1 dup !s2 !s3 ;
  :rotl (x--) !k [ @k n:negate shift ] [ #32 @k - shift ] bi or ;
  :res** (--n) @s0 #5 * #7 rotl #9 * ;
---reveal---
  :random:xoroshiro128**  (-n)
    @s1 #-9 shift !t
    @s0 @s2 xor !s2
    @s1 @s3 xor !s3
    @s2 @s1 xor !s1
    @s3 @s0 xor !s0
    @t  @s2 xor !s2
    @s3 #11 rotl !s3 
    res** ;
  :random:xoroshiro128**:set-seed (n-)
    seed #100 [ random:xoroshiro128** drop ] times ;
}}
~~~
