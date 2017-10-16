# rosetta|is-palindrome

A palindrome is a phrase which reads the same backward and forward.

Write a function or program that checks whether a given sequence of characters (or, if you prefer, bytes) is a palindrome.


In Retro this is fairly easy. We can use `s:hash` to identify a unique string. So make a copy, take he hash, reverse the copy, get its hash, and compare them.

````
:s:palindrome? (s-f)
  dup s:hash [ s:reverse s:hash ] dip eq? ;

'ingirumimusnocteetconsumimurigni s:palindrome?
````