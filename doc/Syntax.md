# Syntax

RETRO code consists of a series of whitespace delimited tokens. Each of these can have an optional prefix telling RETRO how to treat the token. If the token lacks a valid prefix, it will be treated as a word name.

So:

    [prefix][token]

Prefixes are single character modifiers. They are similar to colors in ColorForth, but are handled via words in the `prefix:` namespace.

The major prefixes are:

| Prefix | Used For                      |
| ------ | ----------------------------- |
| @      | Fetch from variable           |
| !      | Store into variable           |
| &      | Pointer to named item         |
| #      | Numbers                       |
| $      | ASCII characters              |
| '      | Strings                       |
| (      | Comments                      |
| :      | Define a word                 |

Example:

````
(This_is_a_comment)

(Define_some_words)
:hello  (-)
  'Hello_World! puts nl ;

:n:square  (n-m)
  dup * ;

(Use_them)
hello
#33 n:square

(More_Things)
$a (this_is_the_ASCII_'a')

'Use_underscores_in_place_of_spaces_in_strings
&puts call

'Foo var
#100 !Foo
@Foo putn
````
