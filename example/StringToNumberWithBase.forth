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
