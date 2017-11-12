#!/usr/bin/env rre

This will (hopefully) be a functional text editor written in RETRO.
It draws influence from my earlier block editors, but is intended to
operate on actual text files instead of blocks.

First up, several variables and constants that are used through
the rest of the code.

~~~
'SourceFile var
'CurrentLine var
'FID var
~~~

~~~
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

  <filename> : <line-count> : <current-line>
  ---------------------------------------------------------------
  *   99:
     100: :n:square dup * ;
     101:
     102: This is the current line
     103:
  ---------------------------------------------------------------

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

~~~
:clear-display (-)
  ASCII:ESC '%c[2J s:with-format puts nl ;

:---- (-)
  #78 [ $- putc ] times nl ;

:header (-)
  @CurrentLine count-lines @SourceFile '%s_:_%n_:_%n\n s:with-format puts ;

:pad (n-n)
  dup #0 #9 n:between? [ '___ puts ] if
  dup #10 #99 n:between? [ '__ puts ] if
  dup #100 #999 n:between? [ '_ puts ] if ;

:mark-if-current (n-n)
  dup @CurrentLine eq? [ '*_ puts ] [ '__ puts ] choose ; 

:line# (n-)
  putn ':_ puts ;

:display-line (n-n)
  dup mark-if-current pad line# n:inc @FID file:read-line puts nl ;

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

~~~
:current? (n-nf)
  over @CurrentLine eq? ;

:delete-line (-)
  TEMP-FILE file:W file:open !FID
  #0 @SourceFile [ current? [ drop s:empty ] if file:puts n:inc ] file:for-each-line drop
  @FID file:close
  @SourceFile TEMP-FILE 'mv_%s_%s s:with-format unix:system ;
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
  TEMP-FILE file:W file:open !FID
  #0 @SourceFile [ current? [ drop gets ] if file:puts n:inc ] file:for-each-line drop
  @FID file:close
  @SourceFile TEMP-FILE 'mv_%s_%s s:with-format unix:system ;
~~~

And now tie everything together. There's a key handler and a top level loop.

~~~
:help
  'j-down_|_k-up_|_i-replace_|_d-delete puts nl ;

:handler
    getc
      $i [ replace-line                           ] case
      $d [ delete-line                            ] case
      $j [ &CurrentLine v:inc                     ] case
      $k [ &CurrentLine v:dec                     ] case
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
