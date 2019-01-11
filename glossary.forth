#!/usr/bin/env retro

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
field. I use `skip` to implement `select`, which selects
a specific field.

~~~
:select   (n-s)
  @SourceLine swap skip ASCII:HT s:split nip ;
~~~

And then named words to access each field.

~~~
:field:name   (-s) #0 select ;
:field:dstack (-s) #1 select ;
:field:astack (-s) #2 select ;
:field:fstack (-s) #3 select ;
:field:descr  (-s) #4 select ;
:field:itime  (-s) #5 select ;
:field:ctime  (-s) #6 select ;
:field:class  (-s) #7 select ;
:field:ex1    (-s) #8 select ;
:field:ex2    (-s) #9 select ;
:field:namespace (-s) #10 select ;
:field:interface (-s) #11 select ;
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

Note to self: This is horribly messy and should be rewritten.

~~~
:s:put<formatted> (s-)  s:format s:put ;

:display-result
  field:name   s:put nl nl
  field:dstack '__Data:__%s\n s:put<formatted>
  field:astack '__Addr:__%s\n s:put<formatted>
  field:fstack '__Float:_%s\n s:put<formatted> nl
  field:descr  '%s\n\n        s:put<formatted>
  field:itime s:length n:zero? [ field:itime 'Interpret_Time:\n__%s\n\n s:put<formatted> ] -if
  field:ctime s:length n:zero? [ field:ctime 'Compile_Time:\n__%s\n\n   s:put<formatted> ] -if
  field:interface field:namespace field:class
  'Class_Handler:_%s_|_Namespace:_%s_|_Interface_Layer:_%s\n\n s:put<formatted>
  field:ex1 '{n/a} s:eq? [ field:ex1 s:format 'Example_#1:\n\n%s\n\n s:put<formatted> ] -if
  field:ex2 '{n/a} s:eq? [ field:ex2 s:format 'Example_#2:\n\n%s\n\n s:put<formatted> ] -if ;
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
  TARGET '%s\t-\t-\t-\t{n/a}\t\t\tclass:word\t{n/a}\t{n/a}\t{n/a}\t{n/a}\t{n/a}\n s:format
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

And provide a word like `s:put` that writes to this:

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
  '%s\t%s\t%s\t%s\t s:format write-line
  s:empty [ '/tmp/glossary.class     file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.ctime     file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.itime     file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.descr     file:slurp ] sip clean s:keep
  '%s\t%s\t%s\t%s\t s:format write-line
  s:empty [ '/tmp/glossary.interface file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.namespace file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.ex2       file:slurp ] sip clean s:keep
  s:empty [ '/tmp/glossary.ex1       file:slurp ] sip clean s:keep
  '%s\t%s\t%s\t%s\t s:format write-line ;
~~~

Next, get the editor from the $EDITOR environment variable.

~~~
'EDITOR s:empty [ unix:getenv ] sip 'EDITOR s:const
~~~

~~~
:edit:field (s-)
  EDITOR '%s_/tmp/glossary.%s s:format unix:system ;
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
  [ s:keep !SourceLine display-result #64 [ $- c:put ] times nl nl ] file:for-each-line ;
~~~

### TSV

I also provide for exporting the tab separated file itself. This will
strip out fields beyond the standard set, which can save some space if
you edit/save the TSV data with a spreadsheet application.

~~~
:display-fields
  field:name s:put tab
  field:dstack s:put tab
  field:astack s:put tab
  field:fstack s:put tab
  field:descr s:put tab
  field:itime s:put tab
  field:ctime s:put tab
  field:class s:put tab
  field:ex1 s:put tab
  field:ex2 s:put tab
  field:namespace s:put tab
  field:interface s:put tab
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
  'RETRO_Glossary_Tool s:put nl
  #32 [ $- c:put ] times nl
  'describe_<wordname> s:put nl
  'delete_<wordname> s:put nl
  'add_<wordname> s:put nl
  'edit_<field>_<wordname> s:put nl
  'export_<format> s:put nl
  #32 [ $- c:put ] times nl
  'Editor_Fields: s:put nl
  '__name\n__dstack\n__astack\n__fstack\n s:format s:put
  '__descr\n__itime\n__ctime\n__class\n s:format s:put
  '__ex1\n__ex2\n__namespace\n__interface\n s:format s:put
  #32 [ $- c:put ] times nl
  'Export_Formats: s:put nl
  '__glossary s:put nl
  '__tsv      s:put nl
  #32 [ $- c:put ] times nl

;
~~~

# Gopher and HTTP Server

This tool embeds a tiny Gopher and HTTP server designed to run
under inetd.

First, set the port to use. I default to 9999.

~~~
#9999 'GOPHER-PORT const
~~~

Next, words to display the main index (when requesting / or an
empty selector).

Gopher protocol for directories dictates the following format:

    <type><description>\t<selector>\t<server>\t<port>\r\n

So `display-entry` constructs these. The selectors chosen are
`desc wordname`; the server is hardcoded to forthworks.com in
this.

~~~
:display-entry (-)
  GOPHER-PORT field:name dup '0%s\t/desc_%s\tforthworks.com\t%n\r\n s:format s:put ;
~~~

Next, `gopher:list-words` which iterates over each entry,
generating the directory line for each.

~~~
:gopher:list-words (-)
  'words.tsv [ s:keep !SourceLine display-entry ] file:for-each-line ;
~~~

With the Gopher side of the index taken care of, I turn my
attentions towards HTTP. In this case, the index is an HTML
file with a bunch of hyperlinks. Since we can't just pass
any non-whitespace in the URLs, this uses the line number in
**words.tsv** instead.

As with the Gopher, there's a `display-entry` which makes
the HTML for each line, and an `http:list-words` which uses
this to build an index.

~~~
:sanitize (s-s)
  here buffer:set
  [ $< [ '&lt;  [ buffer:add ] s:for-each ] case
    $> [ '&gt;  [ buffer:add ] s:for-each ] case
    $& [ '&amp; [ buffer:add ] s:for-each ] case
    buffer:add ] s:for-each buffer:start s:temp ;

:display-entry (n-n)
  field:name sanitize over '<a_href="/%n">%s</a><br> s:format s:put ;

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

:s:get (a-)
  buffer:set [ c:get dup buffer:add eol? not ] while ;

'Selector d:create
  #1024 allot

:css (-)
  '<style>tt,_a,_pre,_xmp_{_white-space:_pre;_} s:put
  '_*_{_font-family:_monospace;_color:_#aaa;_background:_#121212;_font-size:_large;_}_a_{_color:_#EE7600;_} 
s:put
  '</style> s:put nl ;

:http:display (-)
  #0 'words.tsv [ s:keep !SourceLine dup-pair eq? [ '<xmp> s:put display-result '</xmp> s:put ] if n:inc ] file:for-each-line drop-pair ;

:handle-http
  css
  '<h2><a_href="http://forthworks.com:9999">RETRO_Glossary</a></h2><hr> s:put nl
  &Selector ASCII:SPACE s:tokenize #1 set:nth fetch
  dup s:length #1 eq?
    [ drop http:list-words ]
    [ n:inc s:to-number http:display ] choose ;

:gopher:serve
  &Selector s:get
  &Selector #0 #5 s:substr
  '/desc [ &Selector ASCII:SPACE s:tokenize #1 set:nth fetch s:chop s:keep !Target gopher:display ] s:case
  'GET_/ [ 'HTTP/1.0_200_OK\nContent-Type:_text/html\n\n s:format s:put handle-http ] s:case
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
