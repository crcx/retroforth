#!/usr/bin/env rre

# Hua: a text editor written in RETRO

Hua is a small, functional text editor written in RETRO, using the
*RRE* interface. It is line oriented, visual, and tries to be very
simple to use.

First up, several variables and constants that are used through
the rest of the code.

~~~
'SourceFile var
'CurrentLine var
'FID var
~~~

The configuration here is for two items. The number of lines from the
file to show on screen, and the name of the temporary file to use
when editing.

~~~
#80 'COLS      const
#12 'MAX-LINES const
'/tmp/rre.edit 'TEMP-FILE s:const
~~~

Get the name of the file to edit. If no file is provided, exit.

~~~
sys:argc n:zero? [ #0 unix:exit ] if
#0 sys:argv s:keep !SourceFile
~~~

This is just a shortcut to make writing strings to the current file
easier.

~~~
:file:puts (s-) [ @FID file:write ] s:for-each ASCII:LF @FID file:write ;
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
  #0 @SourceFile [ drop n:inc ] file:for-each-line ;

:skip-to
  @CurrentLine #0 n:max [ @FID file:read-line drop ] times ;
~~~

Now for words to format the output. This should all be pretty clear in
intent.

`clear-display` uses an ANSI/VT100 escape sequence. This might need to
be adjusted for your chosen terminal.

~~~
:clear-display (-)
  ASCII:ESC '%c[2J s:with-format puts nl ;
~~~

This just displays the separator bars.

~~~
:---- (-)
  COLS [ $- putc ] times nl ;
~~~

Next, a word to display the header. Currently just the name of the file
being edited and the line count.

~~~
:header (-)
  count-lines @SourceFile '%s_:_%n_lines\n s:with-format puts ;
~~~

The `pad` word is used to make sure line numbers are all the same width.

~~~
:pad (n-n)
  dup #0 #9 n:between? [ '____ puts ] if
  dup #10 #99 n:between? [ '___ puts ] if
  dup #100 #999 n:between? [ '__ puts ] if
  dup #1000 #9999 n:between? [ '_ puts ] if ;
~~~

A line has a form:

    <indicator><number>: <text>

The indicator is an asterisk, and visually marks the current line.

~~~
:mark-if-current (n-n)
  dup @CurrentLine eq? [ $* putc ] [ sp ] choose ; 

:line# (n-)
  putn ':_ puts ;

:display-line (n-n)
  dup mark-if-current pad line# n:inc @FID file:read-line puts nl ;
~~~

~~~
:display (-)
  @SourceFile file:R file:open !FID
  clear-display header ---- skip-to
  @CurrentLine count-lines MAX-LINES n:min [ display-line ] times drop
  ---- dump-stack
  @FID file:close ;
~~~

With the code to display the file done, I can proceed on to words for
handling editing. First, is a word to delete contents of the current
line.

The process here is to just write all but the current line to a dummy
file, replacing the current line text with a newline. Then replace the
original file with the dummy one.

I add a custom combinator, `process-lines` to iterate over the lines in
the file. This takes a quote, and runs it once for each line in the file.
The quote gets passed two values: a counter and a pointer to the current
line in the file. The quote should consume the pointer an increment the
counter. This also sets up `FID` as a pointer to the temporary file where
changes can be written. The combinator will replace the original file
after execution completes.

~~~
:process-lines (q-)
  TEMP-FILE file:W file:open !FID
  [ #0 @SourceFile ] dip file:for-each-line drop
  @FID file:close
  @SourceFile TEMP-FILE 'mv_%s_%s s:with-format unix:system ;
~~~

~~~
:current? (n-nf)
  over @CurrentLine eq? ;

:delete-line (-)
 [ current? [ drop s:empty ] if file:puts n:inc ] process-lines ;
~~~

~~~
:kill-line (-)
  [ current? [ drop ] [ file:puts ] choose n:inc ] process-lines ;
~~~

~~~
:add-line (-)
  [ current? [ ASCII:LF @FID file:write ] if file:puts n:inc ] process-lines ;
~~~

Replacing a line is next. Much like the `delete-line`, this writes all
but the current line to a dummy file. It uses a `gets` word to read in
the text to write instead of the original current line. When done, it
replaces the original file with the dummy one.

~~~
:gets (-s)
  s:empty [ buffer:set
    [ repeat getc dup ASCII:LF -eq? 0; drop buffer:add again ] call drop ] sip ;

:replace-line (-)
  [ current? [ drop gets ] if file:puts n:inc ] process-lines ;
~~~

~~~
:goto (-)
  gets s:to-number !CurrentLine ;
~~~

And now tie everything together. There's a key handler and a top level loop.

~~~
:| '_|_ puts ;
:describe (cs-)
  swap putc $: putc puts ;

:help
  $1 'insert_line describe | $2 'replace_text describe | $3 '____________ describe |
  $4 'erase_text_ describe | $5 'delete_line_ describe nl
  $j 'down_______ describe | $k 'up__________ describe | $g 'goto_line___ describe | 
  #32 '___________ describe | $q 'quit________ describe nl ;

:handler
    getc
      $1 [ add-line                               ] case
      $2 [ replace-line                           ] case
      $4 [ delete-line                            ] case
      $5 [ kill-line                              ] case
      $j [ &CurrentLine v:inc &CurrentLine #0 #10000 v:limit ] case
      $k [ &CurrentLine v:dec &CurrentLine #0 #10000 v:limit ] case
      $g [ goto &CurrentLine #0 #10000 v:limit    ] case
      $q [ 'stty_-cbreak unix:system #0 unix:exit ] case
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
