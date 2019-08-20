A standard RETRO image only supports decimal base numbers. This
shows a way to add support for multiple bases.

I begin by creating a variable for the current base and words to
set this to specific values quickly.

~~~
#10 'Base var<n>

:decimal #10 !Base ;
:binary  #2  !Base ;
:octal   #8  !Base ;
:hex     #16 !Base ;
~~~

Next, a string constant with the symbols for each digit. Note that
I'm only going to support uppercase for hexadecimal.

~~~
'0123456789ABCDEF 'DIGITS s:const
~~~

So conversion to a numeric value is pretty easy. The basic idea
here is:

- set a variable to hold the numeric value to zero (as a stating point)
- check to see if the first character is - for negative, set a modifier
- for each character in the string:

  - convert to a numeric value (in this case, it's the index in the
    DIGITS string)
  - Multiply the last value of the number accumulator by the base
  - Add the converted value
  - Store the result in the number accumulator

- Multiply the final number by the modifier

~~~
{{
  'Number var
  'Mod    var
  :convert    (c-)  &DIGITS swap s:index-of @Number @Base * + !Number ;
  :check-sign (s-s) dup fetch $- eq? [ #-1 !Mod n:inc ] [ #1 !Mod ] choose ;
---reveal---
  :s:to-number<with-base> (s-n)
    #0 !Number check-sign [ convert ] s:for-each @Number @Mod * ;
}}
~~~

Going the other way, back to a string, follows a similar process.

- Take a value
- Repeat:
  - Divide by `Base`
  - Convert result to character and append to a buffer
  - If remainder is not zero, repeat
- If number is negative, append the '-' symbol to the buffer
- Reverse the buffer contents to return a string in the correct order

~~~
{{
  'String d:create   #12 allot
  :check-sign (n-)   n:negative? [ $- buffer:add ] if ;
  :n->digit   (n-c)  &DIGITS + fetch ;
  :convert    (n-)   [ @Base /mod swap n->digit buffer:add dup n:zero? ] until drop ;
---reveal---
  :n:to-string<with-base> (n-s)
    [ &String buffer:set dup n:abs convert check-sign ] buffer:preserve
    &String s:reverse ;
}}
~~~

The `n:to-string<with-base>` returns a representation of binary numbers, but
not the actual bitwise representation. The next word takes care of this.

~~~
{{
  'Selector var
  :bit (f-c) [ $0 ] [ $1 ] choose ;
---reveal---
  :n:binary-rep (n-s)
    dup n:negative? [ n:inc &n:odd? ] [ &n:even? ] choose !Selector
    [ s:empty buffer:set
      #32 [ dup @Selector call bit buffer:add #2 / ] times drop
      buffer:start s:reverse
    ] buffer:preserve ;
}}
~~~

