#!/usr/bin/env rre

This is an application for looking up documentation in the words.tsv
data set.

It's intended to be used like:

    ./glossary.forth describe s:filter

Or:

    ./glossary.forth export glossary

First, exit if the required number of argumenta are not passed.

~~~
sys:argc #2 -eq? [ #0 unix:exit ] if
~~~

If we made it this far, at least two arguments were passed. This
assumes the first argument is the type of search and the second
is name of a word, class, or other detail to search for. Grab them
and save as `QUERY` and `TARGET`.

~~~
#0 sys:argv 'QUERY s:const
#1 sys:argv 'TARGET s:const
~~~

The *words.tsv* has the following fields:

    | name                       | 0
    | data stack                 | 1
    | address stack              | 2
    | float stack                | 3
    | general description        | 4
    | interpret time description | 5
    | compile time description   | 6
    | class handler              | 7
    | example 1                  | 8
    | example 2                  | 9
    | namespace                  | 10
    | interface                  | 11

So 11 fields. These are stored, one line per entry, with the fields as
tab separated data.

~~~
'SourceLine var

:skip (n-) [ ASCII:HT s:split drop n:inc ] times ;

:field:name   (-s) @SourceLine
                   ASCII:HT s:split nip ;

:field:dstack (-s) @SourceLine #1 skip
                   ASCII:HT s:split nip ;

:field:astack (-s) @SourceLine #2 skip
                   ASCII:HT s:split nip ;

:field:fstack (-s) @SourceLine #3 skip
                   ASCII:HT s:split nip ;

:field:descr  (-s) @SourceLine #4 skip
                   ASCII:HT s:split nip ;

:field:itime  (-s) @SourceLine #5 skip
                   ASCII:HT s:split nip ;

:field:ctime  (-s) @SourceLine #6 skip
                   ASCII:HT s:split nip ;

:field:class  (-s) @SourceLine #7 skip
                   ASCII:HT s:split nip ;

:field:ex1    (-s) @SourceLine #8 skip
                   ASCII:HT s:split nip ;

:field:ex2    (-s) @SourceLine #9 skip
                   ASCII:HT s:split nip ;

:field:namespace (-s) @SourceLine #10 skip
                      ASCII:HT s:split nip ;

:field:interface (-s) @SourceLine #11 skip
                      ASCII:HT s:split nip ;
~~~

~~~
:display-result
  field:name puts nl nl
  field:dstack '__Data:__%s\n s:with-format puts
  field:astack '__Addr:__%s\n s:with-format puts
  field:fstack '__Float:_%s\n s:with-format puts nl
  field:descr s:with-format puts nl nl
  field:itime s:length n:zero? [ 'Interpret_Time:\n__ s:with-format puts field:itime s:with-format puts nl nl ] -if
  field:ctime s:length n:zero? [ 'Compile_Time:\n__ s:with-format puts field:ctime s:with-format puts nl nl ] -if
  field:interface field:namespace field:class
  'Class_Handler:_%s_|_Namespace:_%s_|_Interface_Layer:_%s\n\n
  s:with-format puts
  field:ex1 '{n/a} s:eq? [ 'Example_#1: puts nl field:ex1 s:with-format puts nl nl ] -if
  field:ex2 '{n/a} s:eq? [ 'Example_#2: puts nl field:ex2 s:with-format puts nl nl ] -if ;
~~~

~~~
QUERY 'describe s:eq?
[ 'words.tsv
  [ s:keep !SourceLine field:name TARGET s:eq? [ display-result ] if ] file:for-each-line
] if

QUERY 'export s:eq? 'glossary TARGET s:eq? and
[ 'words.tsv
  [ s:keep !SourceLine display-result #64 [ $- putc ] times nl nl ] file:for-each-line
] if
~~~

~~~
:display-fields
  field:name puts tab
  field:dstack puts tab
  field:astack puts tab
  field:fstack puts tab
  field:descr puts tab
  field:itime puts tab
  field:ctime puts tab
  field:class puts tab
  field:ex1 puts tab
  field:ex2 puts tab
  field:namespace puts tab
  field:interface puts tab
  nl ;

QUERY 'export s:eq? 'tsv TARGET s:eq? and
[ 'words.tsv
  [ s:keep !SourceLine display-fields ] file:for-each-line
] if

~~~
