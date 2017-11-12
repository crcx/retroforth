#!/usr/bin/env rre

This will (hopefully) be a functional text editor written in RETRO.

Interface:

  <filename> : <line-count> : <current-line>
  ---------------------------------------------------------------
  *   99:
     100: :n:square dup * ;
     101:
     102: This is the current line
     103:
  ---------------------------------------------------------------

As with my old block editor, this will be primarily line oriented.
And it's visual, with some simple key bindings.

~~~
'SourceFile var
'FID var
'FOUT var

#0 sys:argv s:keep !SourceFile

:count-lines #0 @SourceFile [ drop n:inc ] file:for-each-line ;

#12 'MAX-LINES const
'CurrentLine var

:pad (n-n)
  dup #0 #9 n:between? [ '___ puts ] if
  dup #10 #99 n:between? [ '__ puts ] if
  dup #100 #999 n:between? [ '_ puts ] if ;

:current (n-n)
  dup @CurrentLine eq? [ '*_ puts ] [ '__ puts ] choose ; 

:skip-to
  @CurrentLine #0 n:max [ @FID file:read-line drop ] times ;

:display
  @SourceFile file:R file:open !FID
  skip-to
  @CurrentLine count-lines MAX-LINES n:min [ dup current pad putn n:inc ':_ puts @FID file:read-line puts nl ] times drop
  @FID file:close ;

:delete-line
  '/tmp/rre.edit file:W file:open !FID
  #0 @SourceFile [ over @CurrentLine eq? [ drop ] [ [ @FID file:write ] s:for-each ] choose ASCII:LF @FID file:write n:inc ] file:for-each-line drop
  @FID file:close
  @SourceFile 'mv_/tmp/rre.edit_%s s:with-format unix:system ;

:handler
    getc
      $d [ delete-line                            ] case
      $j [ &CurrentLine v:inc                     ] case
      $k [ &CurrentLine v:dec                     ] case
      $q [ 'stty_-cbreak unix:system #0 unix:exit ] case
    drop ;

:loop
  'stty_cbreak unix:system
  repeat
    ASCII:ESC '%c[2J s:with-format puts nl
    @CurrentLine count-lines @SourceFile '%s_:_%n_:_%n\n s:with-format puts
    display
    #72 [ $- putc ] times nl
    handler
  again ;

loop
~~~
