#!/usr/bin/env rre

# Overview

This is an application for looking up and updating documentation for
the words provided by RETRO.

# Data Set

I like plain text formats, so the data is stored as a text file, with
one line per word. Each line has a number of fields. These are tab
separated. The fields are:

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

I use a variable named `SourceLine` to point to the current line
contents.

~~~
'SourceLine var
~~~

And a helper word to skip a specified number of fields.

~~~
:skip (n-) [ ASCII:HT s:split drop n:inc ] times ;
~~~

Then it's easy to add words to return each individual field.

~~~
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

# Display an Entry

I implement a word to display an entry. This will use a format like:

    name

      Data:  -
      Addr:  -
      Float: -

    A description of the word.

    Class Handler: class:word | Namespace: global | Interface Layer: all

If there are specific notes on interpret or compile time actions, or any
examples, they will be displayed after the description.

~~~
:puts<formatted> (s-)  s:with-format puts ;

:display-result
  field:name puts nl nl
  field:dstack '__Data:__%s\n puts<formatted>
  field:astack '__Addr:__%s\n puts<formatted>
  field:fstack '__Float:_%s\n puts<formatted> nl
  field:descr  '%s\n\n        puts<formatted>
  field:itime s:length n:zero? [ field:itime 'Interpret_Time:\n__%s\n\n puts<formatted> ] -if
  field:ctime s:length n:zero? [ field:ctime 'Compile_Time:\n__%s\n\n   puts<formatted> ] -if
  field:interface field:namespace field:class
  'Class_Handler:_%s_|_Namespace:_%s_|_Interface_Layer:_%s\n\n puts<formatted>
  field:ex1 '{n/a} s:eq? [ field:ex1 s:with-format 'Example_#1:\n%s\n\n puts<formatted> ] -if
  field:ex2 '{n/a} s:eq? [ field:ex2 s:with-format 'Example_#2:\n%s\n\n puts<formatted> ] -if ;
~~~

# Begin Processing

This application can take a variable number of arguments.

I first check to make sure at least one was passed. If not, just exit.

~~~
sys:argc n:zero? [ #0 unix:exit ] if
~~~

If execution reaches this point there's at least one argument. I use a
loop to store arguments into an array.

~~~
'Args d:create #32 allot
#0 sys:argc [ dup sys:argv s:keep over &Args + store n:inc ] times drop
~~~

And then populate constants for each one I care about.

~~~
#0 &Args + fetch 'QUERY s:const
#1 &Args + fetch 'TARGET s:const
#2 &Args + fetch 'TARGET2 s:const 
~~~

FUTURE:

:process-arguments
  QUERY
  'help     [ display-help           ] s:case
  'describe [ find-and-display-entry ] s:case
  'export   [ handle-exports         ] s:case
  'edit     [ handle-edits           ] s:case
  drop ;

----

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


The *words.tsv* has the following fields:


So 11 fields. These are stored, one line per entry, with the fields as
tab separated data.

~~~

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

~~~
:export-fields
  field:name      '/tmp/glossary.name      file:spew
  field:dstack    '/tmp/glossary.dstack    file:spew
  field:astack    '/tmp/glossary.astack    file:spew
  field:fstack    '/tmp/glossary.fstack    file:spew
  field:descr     '/tmp/glossary.descr     file:spew
  field:itime     '/tmp/glossary.itime     file:spew
  field:ctime     '/tmp/glossary.ctime     file:spew
  field:class     '/tmp/glossary.class     file:spew
  field:ex1       '/tmp/glossary.ex1       file:spew
  field:ex2       '/tmp/glossary.ex2       file:spew
  field:namespace '/tmp/glossary.namespace file:spew
  field:interface '/tmp/glossary.interface file:spew ;
~~~

~~~
'EDITOR here [ unix:getenv ] sip s:temp 'EDITOR s:const
'FID var

:edit:description
  '/tmp/glossary.descr file:W file:open !FID
  field:descr [ @FID file:write ] s:for-each @FID file:close
  '/tmp/glossary.descr EDITOR '%s_%s s:with-format unix:system ;

'FOUT var
:fputs (s-) [ dup ASCII:CR eq? over ASCII:LF eq? or [ drop ] [ @FOUT file:write ] choose ] s:for-each ASCII:HT @FOUT file:write ;

:display-fields
  field:name fputs
  field:dstack fputs
  field:astack fputs
  field:fstack fputs
  here '/tmp/glossary.descr file:slurp here fputs
  field:itime fputs
  field:ctime fputs
  field:class fputs
  field:ex1 fputs
  field:ex2 fputs
  field:namespace fputs
  field:interface fputs
  ;


QUERY 'edit:descr s:eq?
[ 'words.tsv
  [ s:keep !SourceLine field:name TARGET s:eq? [ edit:description ] if ] file:for-each-line
  'words.new file:W file:open !FOUT
  'words.tsv
  [ s:keep !SourceLine field:name TARGET s:eq? [ display-fields ] [ @SourceLine [ @FOUT file:write ] s:for-each ASCII:LF @FOUT file:write ] choose ] file:for-each-line
  @FOUT file:close
] if

~~~
