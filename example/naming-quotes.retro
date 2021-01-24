# Naming Quotes

Anonymous functions called quotes are used heavily by Retro.
This shows a way to attach names to them.

In a classic Forth, words are created using `:`, which is
one of numerous parsing words. So a named function looks
like:

    : foo ... ;

In Retro, there are no parsing words. There is a prefix
handler `:`, yielding:

    :foo ... ;

Quotes start with `[` and end with `]`. So they look like:

    [ ... ]

If we want to name a quote, we need to:

- create a header
- assign the xt field to the quote address
- set the class handler

This word, `def`, does these.

~~~
:def (as-)  d:create  d:last d:xt store  &class:word reclass ;
~~~

An example of using this:

```
[ 'Hello_%s!\n s:format s:put ]  'hello def
'#forth hello
```
