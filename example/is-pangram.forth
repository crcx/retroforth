A pangram is a scentence that uses all the letters in the alphabet.

This is one way to determine if a scentence is a pangram using RETRO.

First, define a string containing the alphabet:

~~~
'abcdefghijklmnopqrstuvwxyz 'FULL s:const
~~~

Then a blank string of the same length for the test data:

~~~
'__________________________ 'TEST s:const
~~~

Now a word to do the actual test.

~~~
:s:pangram? (s-f)
  '__________________________ &TEST #26 copy
  s:to-lower [ c:letter? ] s:filter
  [ dup $a - &TEST + store ] s:for-each
  &TEST &FULL s:eq? ;
~~~

Breaking this down, the first line:

  '__________________________ &TEST #26 copy

Copies a blank string over the TEST string. Then:

  s:to-lower [ c:letter? ] s:filter

Converts the string to lowercase and strips out anything that's not a
letter. Then a quick iteration over each character:

  [ dup $a - &TEST + store ] s:for-each

Reduces the letter to an index into the TEST string and stores the
letter at the appropriate spot. And finally:

  &TEST &FULL s:eq? ;

Just compares the TEST and FULL strings to get the result.

Here's a couple of test cases:

~~~
'Hello_world! s:pangram?
'The_quick_brown_fox_jumped_over_the_lazy_dogs. s:pangram?
~~~
