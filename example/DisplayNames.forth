Forth systems often have a word that displays the names of all
words in the dictionary. This can be helpful on occasion, but it
doesn't provide very much information and can be quite daunting
to newer users.

In Retro, I want to do better. The first step towards this is to
allow listing words in different ways. Retro includes two for
doing this:


    word              stack  description
    ================  =====  ===================================
    d:words             -    display all words in the dictionary
    d:words-with       s-    display words with the specified
                             substring in their name

In this, I propose two additional ones:

    word              stack  description
    ================  =====  ===================================
    d:words-in-class   a-    display words with the specified
                             class handler
    d:words-by-class    -    display all words, grouped by their
                             class handler

The first is pretty easy. Retro has a `d:for-each` combinator
for iterating over the dictionary. With this I can pass a pointer
to the class handler and compare this to each header, showing
only the entries that match.

I am using `reorder` to setup the stack. I could also have done
this via `[ over ] dip swap` or `push over pop swap`. The use of
`reorder` is simply done to make the stack alteration obvious.

~~~
:d:words-in-class (a-)
  [ dup d:class fetch
    'abc 'abca reorder
    eq? [ d:name s:put sp ] &drop choose ] d:for-each
  drop ;
~~~

The second one is a little more involved.

First, all class handlers must be identified. I do this with the
`d:for-each` combinator, looking for names that have a `class:`
prefix. (This suffices as I keep all my class handlers in the
`class:` namespace; it won't pick up ones with non-standard naming)

I iterate over the dictionary and construct a set to hold a pointer
to the header for each class I identify.

The exposed word also uses `d:for-each`, iterating over the
dictionary once for each class in the set. It displays the name of
the class, and then uses `d:words-in-class` to display the words.

Of note here, creating a set consumes space. I'm using the
`v:preserve` combinator to preserve the location of `here` so that
any used space is recovered.

~~~
{{
  :identify-classes (-a)
    here #0 ,
    [ dup d:name 'class: s:contains-string?
      &, &drop choose ] d:for-each
    here over - n:dec over store ;
---reveal---
  :d:words-by-class
    &Heap [ identify-classes
            [ [ d:name s:put nl ]
              [ d:xt fetch d:words-in-class ] bi
              nl nl ] array:for-each ] v:preserve ;
}}
~~~


```
d:words-by-class
```
