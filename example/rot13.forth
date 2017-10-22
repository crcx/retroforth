ROT13 ("rotate by 13 places", sometimes hyphenated ROT-13) is a simple
letter substitution cipher that replaces a letter with the letter 13
letters after it in the alphabet. ROT13 is a special case of the Caesar
cipher, developed in ancient Rome.

Because there are 26 letters (21^3) in the basic Latin alphabet, ROT13
is its own inverse; that is, to undo ROT13, the same algorithm is
applied, so the same action can be used for encoding and decoding. The
algorithm provides virtually no cryptographic security, and is often
cited as a canonical example of weak encryption.

(Taken from https://en.m.wikipedia.org/wiki/ROT13)

This is an implementation of ROT13 in RETRO.

~~~
'nopqrstuvwxyzabcdefghijklm 'MAP s:const
:encode (c-c) $a - MAP + fetch ;
:rot13  (s-s) s:to-lower [ dup c:letter? [ encode ] if ] s:map ;
~~~
