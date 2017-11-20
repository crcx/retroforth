#!/usr/bin/env rre

# Overview

This is an application for looking up and updating the
documentation for the words provided by RETRO.

# Data Set

I like plain text formats, so the data is stored as a
text file, with one line per word. Each line has a number
of fields. These are tab separated. The fields are:

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

I use a variable named `SourceLine` to point to the
current line contents.

~~~
'SourceLine var
~~~

And a helper word to skip a specified number of fields.

~~~
:skip (n-) [ ASCII:HT s:split drop n:inc ] times ;
~~~

Then it's easy to add words to return each individual
field.

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

I implement a word to display an entry. This will use a
format like:

    name

      Data:  -
      Addr:  -
      Float: -

    A description of the word.

    Class Handler: class:word | Namespace: global | Interface Layer: all

If there are specific notes on interpret or compile time
actions, or any examples, they will be displayed after
the description.

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
  field:ex1 '{n/a} s:eq? [ field:ex1 s:with-format 'Example_#1:\n\n%s\n\n puts<formatted> ] -if
  field:ex2 '{n/a} s:eq? [ field:ex2 s:with-format 'Example_#2:\n\n%s\n\n puts<formatted> ] -if ;
~~~

# Prepare for Command Line Processing

This application can take a variable number of arguments.

I first check to make sure at least one was passed. If
not, just exit.

~~~
sys:argc n:zero? [ #0 unix:exit ] if
~~~

If execution reaches this point there's at least one
argument. I use a loop to store arguments into an array.

~~~
'Args d:create #32 allot
#0 sys:argc
  [ dup sys:argv s:keep over &Args + store n:inc ] times
  drop
~~~

And then populate constants for each one I care about.

~~~
#0 &Args + fetch 'QUERY s:const
#1 &Args + fetch 'TARGET s:const
#2 &Args + fetch 'TARGET2 s:const 
~~~

# Interactions

With the command line data extracted, I can now move on
to the words for handling specific interactions.

There are five primary roles:

* describe word
* add word
* delete word
* edit word
* export data

## Describe a Word

~~~
:matched? (-f) field:name TARGET s:eq? ;

:find-and-display-entry
 'words.tsv [ s:keep !SourceLine matched? [ display-result ] if ] file:for-each-line ;
~~~

## Add a Word

This just adds a stub to the end of the words.tsv file.
You'll need to run the edit commands for each field to
fully populate it.

~~~
'FADD var

:add-word
  'words.tsv file:A file:open !FADD
  TARGET '%s\t-\t-\t-\t{n/a}\t\t\tclass:word\t{n/a}\t{n/a}\t{n/a}\t{n/a}\t{n/a}\n s:with-format
  [ @FADD file:write ] s:for-each
  @FADD file:close ;
~~~

## Delete a Word

~~~
'FNEW var

:matched? (-f) field:name TARGET s:eq? ;

:delete-entry
  'words.new file:W file:open !FNEW
  'words.tsv [ s:keep !SourceLine matched? [ @SourceLine [ @FNEW file:write ] s:for-each ASCII:LF @FNEW file:write ] -if ] file:for-each-line
  @FNEW file:close
  'mv_words.new_words.tsv unix:system ;
~~~

## Edit a Word

Editing is a bit tricky. To keep things as simple as possible, I export
each field to a separate file under `/tmp/`.

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

Since I'm dumping a bunch of files into `/tmp/`, I also clean up
when done.

~~~
:delete-temporary
  '/tmp/glossary.name      file:delete
  '/tmp/glossary.dstack    file:delete
  '/tmp/glossary.astack    file:delete
  '/tmp/glossary.fstack    file:delete
  '/tmp/glossary.descr     file:delete
  '/tmp/glossary.itime     file:delete
  '/tmp/glossary.ctime     file:delete
  '/tmp/glossary.class     file:delete
  '/tmp/glossary.ex1       file:delete
  '/tmp/glossary.ex2       file:delete
  '/tmp/glossary.namespace file:delete
  '/tmp/glossary.interface file:delete ;
~~~

Cleaning the edited data is necessary. This entails:

- removing any trailing newlines
- converting internal newlines and tabs to escape sequences

~~~
:clean
  dup s:length over + n:dec
    fetch [ ASCII:LF eq? ] [ ASCII:CR eq? ] bi or [ s:chop ] if
  here [ [ ASCII:LF [ $\ , $n , ] case
           ASCII:CR [ $\ , $n , ] case
           ASCII:HT [ $\ , $t , ] case
           ,
         ] s:for-each #0 ,
       ] dip ;
~~~

After an edit, I need to reassemble the pieces and write them out to
the file. I'll use `FOUT` as a variable for the file ID.

~~~
'FOUT var
~~~

And provide a word like `puts` that writes to this:

~~~
:write-line (s-) [ @FOUT file:write ] s:for-each ;
:write-nl   (-)  ASCII:LF @FOUT file:write ;
~~~

~~~
:generate-entry
  s:empty [ '/tmp/glossary.fstack    file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.astack    file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.dstack    file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.name      file:slurp ] sip clean s:keep
  '%s\t%s\t%s\t%s\t s:with-format write-line
  s:empty [ '/tmp/glossary.class     file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.ctime     file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.itime     file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.descr     file:slurp ] sip clean s:keep
  '%s\t%s\t%s\t%s\t s:with-format write-line
  s:empty [ '/tmp/glossary.interface file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.namespace file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.ex2       file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.ex1       file:slurp ] sip clean s:keep
  '%s\t%s\t%s\t%s\t s:with-format write-line ;
~~~

Next, get the editor from the $EDITOR environment variable.

~~~
'EDITOR s:empty [ unix:getenv ] sip 'EDITOR s:const
~~~

~~~
:edit:field (s-)
  EDITOR '%s_/tmp/glossary.%s s:with-format unix:system ;
~~~

~~~
:select-field
  export-fields
  TARGET
  'name      [ 'name      edit:field ] s:case
  'dstack    [ 'dstack    edit:field ] s:case
  'astack    [ 'astack    edit:field ] s:case
  'fstack    [ 'fstack    edit:field ] s:case
  'descr     [ 'descr     edit:field ] s:case
  'itime     [ 'itime     edit:field ] s:case
  'ctime     [ 'ctime     edit:field ] s:case
  'class     [ 'class     edit:field ] s:case
  'ex1       [ 'ex1       edit:field ] s:case
  'ex2       [ 'ex2       edit:field ] s:case
  'namespace [ 'namespace edit:field ] s:case
  'interface [ 'interface edit:field ] s:case
  drop ;
~~~

~~~
:handle-edit
  'words.new file:W file:open !FOUT
  'words.tsv
  [ s:keep !SourceLine field:name TARGET2 s:eq?
    [ select-field generate-entry ] [ @SourceLine write-line ] choose write-nl
  ] file:for-each-line
  @FOUT file:close delete-temporary
  'mv_words.new_words.tsv unix:system ;
~~~

## Export Data

In addition to providing a readable piece of documentation for each word,
I provide the ability to export the data into a few formats.

### Glossary

The glossary file consists of the documentation for each word, with a
separator bar between each entry.

~~~
:export-glossary
 'words.tsv
  [ s:keep !SourceLine display-result #64 [ $- putc ] times nl nl ] file:for-each-line ;
~~~

### TSV

I also provide for exporting the tab separated file itself. This will
strip out fields beyond the standard set, which can save some space if
you edit/save the TSV data with a spreadsheet application.

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

:export-tsv
  'words.tsv [ s:keep !SourceLine display-fields ] file:for-each-line ;
~~~

### Handle Exports

This is a second level command processor for deciding which export format
to use.

~~~
:export-data
  TARGET
  'glossary [ export-glossary ] s:case
  'tsv      [ export-tsv      ] s:case
  drop ;
~~~

## Help

~~~
:show-help
  'RETRO_Glossary_Tool puts nl
  #32 [ $- putc ] times nl
  'describe_<wordname> puts nl
  'delete_<wordname> puts nl
  'add_<wordname> puts nl
  'edit_<field>_<wordname> puts nl
  'export_<format> puts nl
  #32 [ $- putc ] times nl
  'Editor_Fields: puts nl
  '__name\n__dstack\n__astack\n__fstack\n s:with-format puts
  '__descr\n__itime\n__ctime\n__class\n s:with-format puts
  '__ex1\n__ex2\n__namespace\n__interface\n s:with-format puts
  #32 [ $- putc ] times nl
  'Export_Formats: puts nl
  '__glossary puts nl
  '__tsv      puts nl
  #32 [ $- putc ] times nl

;
~~~

# Gopher Server

This tool includes a minimal Gopher server designed to run under inetd.

First, set the port to use. I default to 9999.

~~~
#9999 'GOPHER-PORT const
~~~

Next, words to display the main index (when requesting / or an empty
selector).

~~~
:display-entry (-)
  GOPHER-PORT field:name dup '0%s\tdesc_%s\tforthworks.com\t%n\n s:with-format puts ;

:gopher:list-words (-)
  'words.tsv [ s:keep !SourceLine display-entry ] file:for-each-line ;

:display-entry (-)
  field:name over '<a_href="/%n">%s</a><br> s:with-format puts ;

:http:list-words (-)
  #0 'words.tsv [ s:keep !SourceLine display-entry n:inc ] file:for-each-line drop ;
~~~

Next, words to display a specific word.

~~~
'Target var
:matched? (-f) field:name @Target s:eq? ;

:gopher:display
 'words.tsv [ s:keep !SourceLine matched? [ display-result ] if ] file:for-each-line ;
~~~

And then the actual top level server.

~~~
:eol? (c-f)
  [ ASCII:CR eq? ] [ ASCII:LF eq? ] [ ASCII:HT eq? ] tri or or ;

:gets (a-)
  buffer:set [ getc dup buffer:add eol? not ] while ;

'Selector d:create
  #1024 allot

:css (-)
  '<style>tt,_a,_pre_{_white-space:_pre;_} puts
  '_*_{_font-family:_monospace;_color:_#aaa;_background:_#121212;_font-size:_large;_}_a_{_color:_#EE7600;_} 
puts
  '</style> puts nl ;

:http:display (-)
  #0 'words.tsv [ s:keep !SourceLine dup-pair eq? [ '<pre> puts display-result '</pre> puts ] if n:inc ] file:for-each-line drop-pair ;

:handle-http
  css
  '<h2><a_href="http://forthworks.com:9999">RETRO_Glossary</a></h2><hr> puts nl
  &Selector ASCII:SPACE s:tokenize #1 set:nth fetch
  dup s:length #1 eq?
    [ drop http:list-words ]
    [ n:inc s:to-number http:display ] choose ;

:gopher:serve
  &Selector gets
  &Selector #0 #5 s:substr
  '/desc [ &Selector ASCII:SPACE s:tokenize #1 set:nth fetch s:chop s:keep !Target gopher:display ] s:case
  'GET_/ [ 'HTTP/1.0_200_OK\nContent-Type:_text/html\n\n s:with-format puts handle-http ] s:case
  drop gopher:list-words ;
~~~

# Finish

This checks the command line arguments and calls the proper words to
handle each case.

~~~
:process-arguments
  QUERY
  'describe [ find-and-display-entry ] s:case
  'export   [ export-data            ] s:case
  'edit     [ handle-edit            ] s:case
  'add      [ add-word               ] s:case
  'delete   [ delete-entry           ] s:case
  'serve    [ gopher:serve  ] s:case
  drop show-help ;
~~~

~~~
process-arguments
~~~
