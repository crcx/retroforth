# Sandboxed Dictionaries

This implements some words to create a sandboxed dictionary and
to execute a word or quotation within the sandbox.

# Making A Sandboxed Dictionary

The dictionary is structured as a linked list. To make a new one,
I take an array with the names from the global dictionary, extract
the header fields for each, and make a new list using them.

The `make-dict` will return a pointer to the last entry in the new
dictionary.

~~~
{{
  'D var
  :unpack (d-saa)
    d:lookup [ d:name ] [ d:class fetch ] [ d:xt fetch ] tri ;
  :add-header (saa-)
    here [ @D , , , s, ] dip !D  ;
---reveal---
  :make-dict (a-a)
    #0 !D [ unpack add-header ] a:for-each @D ;
}}
~~~

The `{ ... } make-dict` can be wrapped in something to make this
a little more obvious.

~~~
  :dict{ (-)  |{ ; immediate
  :}dict (-a) |} |make-dict ; immediate
~~~

# Using The Sandboxed Dictionary

I implement a very simple `d:with` to run a quote with a
sandboxed dictionary. This works by temporarily replacing
the global dictionary with the sandboxed one.

~~~
:d:with (qa-)
  &Dictionary [ !Dictionary call ] v:preserve ;
~~~

# A Test Case

This will expose a dictionary with just two words that can be
used with a new `%` prefix.

```
{{
  :swap $A ;
  :dup $B ;
  dict{ 'swap 'dup }dict 'SANDBOX const
---reveal---
  :prefix:% (s) [ s:evaluate c:put ] SANDBOX d:with ;
}}

#70 dup swap
%swap %dup
n:put n:put
nl bye
```
