#!/usr/bin/env retro
     _   _             
    | | | |_   _  __ _ 
    | |_| | | | |/ _` |
    |  _  | |_| | (_| |
    |_| |_|\__,_|\__,_|
    line oriented editor

# Hua: a text editor written in RETRO

Hua is a small, functional text editor written in RETRO for
Unix systems. It is line oriented, visual, and easy to learn.

## Starting

Hua is intended to run as a standalone tool. Use a line like:

    edit.forth filename

To create a new file:

    edit.forth new filename

## A Word of Warning

Hua saves changes as you edit the file. I advise using it along
with a version control system so you can revert changes when
needed.

## The Code

Since this runs as a standalone application I use a quick check
to exit if no arguments were passed.

~~~
sys:argc n:zero? [ 'No_file_specified! s:put nl #0 unix:exit ] if
~~~

If I get here, a filename was provided. So I start by creating
a few variables and constants.

Get the name of the file to edit.

~~~
#0 sys:argv s:keep 'SourceFile var<n>
~~~


The configuration here is for two items. The number of lines
from the file to show on screen, and the name of the temporary
file to use when editing.

~~~
#70 'COLS      const
#16 'MAX-LINES const
'/tmp/hua.editor.scratch 'TEMP-FILE s:const
~~~

Next are the variables that I use to track various bits of
state.

~~~
'CurrentLine var
'LineCount   var
'ShowEOL     var
'FID         var
'CopiedLine  d:create #1025 allot
~~~

To create a new file, Hua allows for the use of `new` followed
by the filename. I handle the file creation here.

~~~
@SourceFile 'new s:eq?
  [ #1 sys:argv s:keep !SourceFile
    @SourceFile file:A file:open file:close ] if
~~~

This is just a shortcut to make writing strings to the current file
easier.

~~~
:file:s:put (s-) [ @FID file:write ] s:for-each ASCII:LF @FID file:write ;
~~~

I now turn my attention to displaying the file. I am aiming for
an interface like:

    <filename> : <line-count>
    ---------------------------------------------------------------
    *   99:
       100: :n:square dup * ;
       101:
       102: This is the current line
       103:
    ---------------------------------------------------------------
    j: down | k: up | ... other helpful text ...

The * denotes the currently selected line.

I start with words to count the number of lines in the file and
advance to the currently selected line.

~~~
:count-lines (-)
  #0 @SourceFile [ drop n:inc ] file:for-each-line dup !LineCount ;

:skip-to
  @CurrentLine MAX-LINES #2 / - #0 n:max [ @FID file:read-line drop ] times ;
~~~

Now for words to format the output. This should all be pretty
clear in intent.

`clear-display` uses an ANSI/VT100 escape sequence. This might
need to be adjusted for your chosen terminal.

~~~
:clear-display (-)
  ASCII:ESC dup '%c[2J%c[0;0H s:format s:put nl ;
~~~

This just displays the separator bars.

~~~
:---- (-)
  #7 [ $- c:put ] times 
  #0 COLS #10 / [ dup n:put #9 [ $- c:put ] times n:inc ] times n:put nl ; 
~~~

Next, a word to display the header. Currently just the name of
the file being edited and the line count.

~~~
:header (-)
  count-lines @SourceFile '%s_:_%n_lines\n s:format s:put ;
~~~

The `pad` word is used to make sure line numbers are all the
same width.

~~~
:pad (n-n)
  dup    #0 #9    n:between? [ '____ s:put ] if
  dup   #10 #99   n:between? [ '___  s:put ] if
  dup  #100 #999  n:between? [ '__   s:put ] if
  dup #1000 #9999 n:between? [ '_    s:put ] if ;
~~~

A line has a form:

    <indicator><number>: <text><eol>

The indicator is an asterisk, and visually marks the current
line.

EOL is optional. If `ShowEOL` is `TRUE`, it'll display a ~ at
the end of each line. This is useful when looking for trailing
whitespace. The indicator can be toggled via the ~ key.

~~~
:mark-if-current (n-n)
  dup @CurrentLine eq? [ $* c:put ] [ sp ] choose ; 

:line# (n-)
  n:put ':_ s:put ;

:eol (-)
  @ShowEOL [ $~ c:put ] if nl ;

:display-line (n-n)
  dup @LineCount lt?
  [ dup mark-if-current pad line# n:inc @FID file:read-line s:put eol ] if ;

:display (-)
  @SourceFile file:R file:open !FID
  clear-display header ---- skip-to
  @CurrentLine MAX-LINES #2 / - #0 n:max count-lines MAX-LINES n:min
  [ display-line ] times drop
  ---- @FID file:close ;
~~~

With the code to display the file done, I can proceed to the
words for handling editing.

I add a custom combinator, `process-lines` to iterate over the
lines in the file. This takes a quote, and runs it once for
each line in the file. The quote gets passed two values: a
counter and a pointer to the current line in the file. The
quote should consume the pointer an increment the counter. This
also sets up `FID` as a pointer to the temporary file where
changes can be written. The combinator will replace the
original file after execution completes.

Additionally, I define a word named `current?` which returns
`TRUE` if the specified line is the current one. This is just
to aid in later readability.

~~~
:process-lines (q-)
  TEMP-FILE file:W file:open !FID
  [ #0 @SourceFile ] dip file:for-each-line drop
  @FID file:close
  here TEMP-FILE file:slurp here @SourceFile file:spew ;

:current? (ns-nsf)
  over @CurrentLine eq? ;
~~~

So first up, a word to delete all text in the current line.

~~~
:delete-line (-)
  [ current? [ drop '_ ] if file:s:put n:inc ] process-lines ;
~~~

Then a word to discard the current line, removing it from the file.

~~~
:kill-line (-)
  [ current? [ drop ] [ file:s:put ] choose n:inc ] process-lines ;
~~~

And the inverse, a word to inject a new line into the file.

~~~
:add-line (-) 
  [ current? [ file:s:put ASCII:LF @FID file:write ] 
             [ file:s:put ] choose n:inc ] process-lines CurrentLine v:inc ;
~~~

Replacing a line is next. Much like the `delete-line`, this
writes all but the current line to a dummy file. It uses an
`s:get` word to read in the text to write instead of the
original current line. When done, it replaces the original
file with the dummy one.

~~~
{{
  :save (c-)
    ASCII:BS  [ buffer:get drop ] case
    ASCII:DEL [ buffer:get drop ] case
    buffer:add ;
---reveal---
  :s:get (-s)  
    s:empty [ buffer:set
      [ repeat c:get dup ASCII:LF -eq? 0; drop save again ] call drop ] sip ;
}}

:replace-line (-) 
  [ current? [ drop #7 [ ASCII:SPACE c:put ] times s:get ] if 
    file:s:put n:inc ] process-lines ;
~~~

The next four are just things I find useful. They allow me to
indent, remove indention, trim trailing whitespace, and insert
a code block delimiter at a single keystroke.

~~~
:indent-line (-)
  [ current? [ ASCII:SPACE dup @FID file:write @FID file:write ] if file:s:put n:inc ] process-lines ;

:dedent-line (-)
  [ current? [ n:inc n:inc ] if file:s:put n:inc ] process-lines ;

:trim-trailing (-)
  [ current? [ s:trim-right ] if file:s:put n:inc ] process-lines ;

:code-block (-)
  [ current? [ drop '~~~ ] if file:s:put n:inc ] process-lines ;
~~~

And then a very limited form of copy/paste, which moves a copy
of the current line into a `CopiedLine` buffer and back again.
The line buffer is 1024 characters long, use of a longer line
will cause problems.

~~~
:copy-line (-)
  [ current? [ dup &CopiedLine s:copy ] if file:s:put n:inc ] process-lines ;

:paste-line (-)
  [ current? [ drop &CopiedLine ] if file:s:put n:inc ] process-lines ;
~~~

One more set of commands: jump to a particular line in the
file, jump to the start or end of the file.

~~~
:goto (-)
  s:get s:to-number !CurrentLine ;

:goto-start (-)
  #0 !CurrentLine ;

:goto-end (-)
  @LineCount n:dec !CurrentLine ;
~~~

And now tie everything together. There's a key handler and a
top level loop.

~~~
:describe (cs-)
  swap c:put $: c:put s:put ;
:| describe '_|_ s:put ;

:help
  $1 'replace_ |
  $2 'insert__ |
  $3 'trim____ |
  $4 'erase___ |
  $5 'delete__ |
  $j 'down____ | nl
  $k 'up______ |
  $g 'goto____ |
  $[ 'start___ |
  $] 'end_____ |
  $c 'copy____ |
  $v 'paste___ | nl
  $< 'dedent__ |
  $> 'indent__ |
  $~ 'mark_eol |
  $| '~~~_____ |
  '___________|_ s:put
  $q 'quit____ | nl ;
~~~

~~~
:constrain (-) &CurrentLine #0 @LineCount n:dec v:limit ;
:handler
    c:get
      $1 [ replace-line                   ] case
      $2 [ add-line                       ] case
      $3 [ trim-trailing                  ] case
      $4 [ delete-line                    ] case
      $5 [ kill-line                      ] case
      $~ [ @ShowEOL not !ShowEOL          ] case
      $c [ copy-line                      ] case
      $v [ paste-line                     ] case
      $< [ dedent-line                    ] case
      $> [ indent-line                    ] case
      $| [ code-block                     ] case
      $[ [ goto-start                     ] case
      $] [ goto-end                       ] case
      $j [ &CurrentLine v:inc constrain   ] case
      $k [ &CurrentLine v:dec constrain   ] case
      $h [ &CurrentLine v:inc constrain   ] case
      $t [ &CurrentLine v:dec constrain   ] case
      $g [ goto               constrain   ] case
      $q [ 'stty_-cbreak unix:system bye  ] case
    drop ;

:edit
  'stty_cbreak unix:system
  repeat
    display help handler
  again ;
~~~

Run the editor.

~~~
edit
~~~
