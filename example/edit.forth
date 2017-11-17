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
'LineCount var
'ShowEOL var
'FID var
'CopiedLine d:create #1025 allot
~~~

The configuration here is for two items. The number of lines from the
file to show on screen, and the name of the temporary file to use
when editing.

~~~
#80 'COLS      const
#16 'MAX-LINES const
'/tmp/rre.edit 'TEMP-FILE s:const
~~~

Get the name of the file to edit. If no file is provided, exit.

~~~
sys:argc n:zero? [ #0 unix:exit ] if
#0 sys:argv s:keep !SourceFile
@SourceFile 'new s:eq?
  [ #1 sys:argv s:keep !SourceFile
    @SourceFile file:A file:open file:close ] if
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
  #0 @SourceFile [ drop n:inc ] file:for-each-line dup !LineCount ;

:skip-to
  @CurrentLine MAX-LINES #2 / - #0 n:max [ @FID file:read-line drop ] times ;
~~~

Now for words to format the output. This should all be pretty clear in
intent.

`clear-display` uses an ANSI/VT100 escape sequence. This might need to
be adjusted for your chosen terminal.

~~~
:clear-display (-)
  ASCII:ESC dup '%c[2J%c[0;0H s:with-format puts nl ;
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

    <indicator><number>: <text><eol>

The indicator is an asterisk, and visually marks the current line.

EOL is optional. If `ShowEOL` is `TRUE`, it'll display a ~ at the end
of each line. This is useful when looking for trailing whitespace. The
indicator can be toggled via the ~ key.

~~~
:mark-if-current (n-n)
  dup @CurrentLine eq? [ $* putc ] [ sp ] choose ; 

:line# (n-)
  putn ':_ puts ;

:eol (-)
  @ShowEOL [ $~ putc ] if nl ;

:display-line (n-n)
  dup @LineCount lt?
  [ dup mark-if-current pad line# n:inc @FID file:read-line puts eol ] if ;

:display (-)
  @SourceFile file:R file:open !FID
  clear-display header ---- skip-to
  @CurrentLine MAX-LINES #2 / - #0 n:max count-lines MAX-LINES n:min [ display-line ] times drop
  ---- @FID file:close ;
~~~

With the code to display the file done, I can proceed on to words for
handling editing.

I add a custom combinator, `process-lines` to iterate over the lines in
the file. This takes a quote, and runs it once for each line in the file.
The quote gets passed two values: a counter and a pointer to the current
line in the file. The quote should consume the pointer an increment the
counter. This also sets up `FID` as a pointer to the temporary file where
changes can be written. The combinator will replace the original file
after execution completes.

Additionally, I define a word named `current?` which returns `TRUE` if
the specified line is the current one. This is just to aid in later
readability.

~~~
:process-lines (q-)
  TEMP-FILE file:W file:open !FID
  [ #0 @SourceFile ] dip file:for-each-line drop
  @FID file:close
  here TEMP-FILE file:slurp here @SourceFile file:spew ;

:current? (n-nf)
  over @CurrentLine eq? ;
~~~

So first up, a word to delete all text in the current line.

~~~
:delete-line (-)
  [ current? [ drop '_ ] if file:puts n:inc ] process-lines ;
~~~

Then a word to discard the current line, removing it from the file.

~~~
:kill-line (-)
  [ current? [ drop ] [ file:puts ] choose n:inc ] process-lines ;
~~~

And the inverse, a word to inject a new line into the file.

~~~
:add-line (-)
  [ current? [ ASCII:LF @FID file:write ] if file:puts n:inc ] process-lines ;
~~~

Replacing a line is next. Much like the `delete-line`, this writes all
but the current line to a dummy file. It uses a `gets` word to read in
the text to write instead of the original current line. When done, it
replaces the original file with the dummy one.

~~~
{{
  :save (c-)
    ASCII:BS  [ buffer:get drop ] case
    ASCII:DEL [ buffer:get drop ] case
    buffer:add ;
---reveal---
  :gets (-s)  
    s:empty [ buffer:set
      [ repeat getc dup ASCII:LF -eq? 0; drop save again ] call drop ] sip ;
}}

:replace-line (-)
  [ current? [ drop gets ] if file:puts n:inc ] process-lines ;
~~~

The next four are just things I find useful. They allow me to indent,
remove indention, trim trailing whitespace, and insert a code block
delimiter at a single keystroke.

~~~
:indent-line (-)
  [ current? [ ASCII:SPACE dup @FID file:write @FID file:write ] if file:puts n:inc ] process-lines ;

:dedent-line (-)
  [ current? [ n:inc n:inc ] if file:puts n:inc ] process-lines ;

:trim-trailing (-)
  [ current? [ s:trim-right ] if file:puts n:inc ] process-lines ;

:code-block (-)
  [ current? [ drop '~~~ ] if file:puts n:inc ] process-lines ;
~~~

And then a very limited form of copy/paste, which moves a copy of the
current line into a `CopiedLine` buffer and back again.

~~~
:copy-line (-)
  [ current? [ dup &CopiedLine s:copy ] if file:puts n:inc ] process-lines ;

:paste-line (-)
  [ current? [ drop &CopiedLine ] if file:puts n:inc ] process-lines ;
~~~

One more set of commands: jump to a particular line in the file, jump
to the start or end of the file.

~~~
:goto (-)
  gets s:to-number !CurrentLine ;

:goto-start (-)
  #0 !CurrentLine ;

:goto-end (-)
  @LineCount n:dec !CurrentLine ;
~~~

And now tie everything together. There's a key handler and a top level loop.

~~~
:describe (cs-)
  swap putc $: putc puts ;
:| describe '_|_ puts ;

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
  '___________|_ puts
  $q 'quit____ | nl ;
~~~

~~~
:constrain (-) &CurrentLine #0 @LineCount n:dec v:limit ;
:handler
    getc
      $1 [ replace-line                           ] case
      $2 [ add-line                               ] case
      $3 [ trim-trailing                          ] case
      $4 [ delete-line                            ] case
      $5 [ kill-line                              ] case
      $~ [ @ShowEOL not !ShowEOL                  ] case
      $c [ copy-line                              ] case
      $v [ paste-line                             ] case
      $< [ dedent-line                            ] case
      $> [ indent-line                            ] case
      $| [ code-block                             ] case
      $[ [ goto-start                             ] case
      $] [ goto-end                               ] case
      $j [ &CurrentLine v:inc constrain           ] case
      $k [ &CurrentLine v:dec constrain           ] case
      $g [ goto               constrain           ] case
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
