# Listing Words in the Dictionary

The basic word listing is provided by `d:words`.

~~~
:d:words    (:-)  [ d:name s:put sp ] d:for-each ;
~~~

This isn't very useful though: a raw list of names is difficult
to read through, and may be intimidating to users. Kiyoshi Yoneda
has implemented some variations that are much more useful.

The first of these is is a variant of `d:words` which displays
words containing a specific substring. It's useful to see words
in a specific namespace, e.g., by doing `'s: d:words-with`, or
words that likely display something: `':put d:words-with`.

~~~
:d:words-with (:s-)
  here s:copy
  [ d:name dup here
    (put-match s:contains/string? [ s:put sp ] [ drop ] choose )
  ] d:for-each ;
~~~

This does have a drawback if you want words in a namespace as
it does not care where in the name the substring is found. To
deal with this, `d:words-beginning-with` is provided.

~~~
{{
  :display-if-left (s-)
    dup here s:begins-with? [ s:put sp ] [ drop ] choose ;
---reveal---
  :d:words-beginning-with (:s-)
    here s:copy [ d:name display-if-left ] d:for-each ;
}}
~~~

~~~
:d:words-missing-details (:-)
  'D.Stack: s:put nl [ dup d:stack fetch n:zero? [ d:name s:put sp ] &drop choose ] d:for-each nl
  'A.Stack: s:put nl [ dup d:astack fetch n:zero? [ d:name s:put sp ] &drop choose ] d:for-each nl
  'F.Stack: s:put nl [ dup d:fstack fetch n:zero? [ d:name s:put sp ] &drop choose ] d:for-each nl
  'Desc:    s:put nl [ dup d:descr fetch n:zero? [ d:name s:put sp ] &drop choose ] d:for-each nl ;
~~~

~~~
'Display_words_missing_one_or_more_information_fields. 'd:words-missing-details d:set-description
'Display_words_with_name_starting_with_the_provided_string. 'd:words-beginning-with d:set-description
'Display_words_with_name_containing_the_provided_string. 'd:words-with d:set-description
'Display_all_named_words. 'd:words d:set-description
~~~
