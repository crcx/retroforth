#!/usr/bin/env retro

~~~
{{
  'Fenced var
  :toggle-fence @Fenced not !Fenced ;
  :fenced? (-f) @Fenced ;
  :handle-line (s-)
    fenced? [ over call ] [ drop ] choose ;
---reveal---
  :unu (sq-)
    swap [ dup '~~~ s:eq?
           [ drop toggle-fence ]
           [ handle-line       ] choose
         ] file:for-each-line drop ;
}}
~~~

~~~
'Image d:create #4096 allot
'AP var
:I, (n-) &Image @AP + store &AP v:inc ;
~~~

## Pass 1

~~~
'Pass_1:_ s:put
#0 !AP
#0 sys:argv
  [ dup s:length n:zero? [ drop #0 ] if 0;
    fetch-next &n:inc dip
    $i [ i here n:dec fetch I, ] case
    $d [ s:to-number I, ] case
    $r [ drop #-1 I, ] case
    $: [ @AP swap 'muri! s:prepend const ] case
    $s [ &I, s:for-each #0 I, ] case
    'ERROR s:put nl
  ] unu
@AP n:put '_cells s:put nl
~~~

## Pass 2

~~~
'Pass_2:_ s:put
#0 !AP
#0 sys:argv
  [ dup s:length n:zero? [ drop #0 ] if 0;
    fetch-next &n:inc dip
    $i [ drop &AP v:inc ] case
    $d [ drop &AP v:inc ] case
    $r [ 'muri! s:prepend d:lookup d:xt fetch I, ] case
    $: [ drop ] case
    $s [ s:length n:inc &AP v:inc-by ] case
    'ERROR s:put nl
  ] unu
@AP n:put '_cells s:put nl
~~~

## Save Image

~~~
'FID var

:write-byte (n-)  @FID file:write ;
:mask       (n-)  #255 and ;

:write-cell (n-)
           dup mask write-byte
  #8 shift dup mask write-byte
  #8 shift dup mask write-byte
  #8 shift     mask write-byte ;

:save-image (s-)
  file:W file:open !FID
  &Image @AP [ fetch-next write-cell ] times drop
  @FID file:close ;

'ngaImage save-image
~~~
