# RETRO

This is a set of extensions for RRE.

# I/O Extensions

# Console Input

~~~
{{
  'io:Keyboard var
  :identify
    @io:Keyboard n:zero? [
      #1 io:scan-for dup n:negative?
      [ drop 'IO_DEVICE_TYPE_0001_NOT_FOUND s:put nl ]
      [ !io:Keyboard ] choose ] if ;
  ---reveal---
  :c:get (-c) identify @io:Keyboard io:invoke ;
}}
~~~

---------------------------------------------------------------

# Scripting: Command Line Arguments

~~~
{{
  'io:Scripting var
  :identify
    @io:Scripting n:zero? [
      #9 io:scan-for dup n:negative?
      [ drop 'IO_DEVICE_TYPE_0009_NOT_FOUND s:put nl ]
      [ !io:Scripting ] choose ] if ;
---reveal---
  :sys:argc (-n)               identify #0 @io:Scripting io:invoke ;
  :sys:argv (n-s) s:empty swap identify #1 @io:Scripting io:invoke ;
  :include  (s-)               identify #2 @io:Scripting io:invoke  ;
}}
~~~

# Interactive Listener

~~~
'NoEcho var

{{
  :version (-)    @Version #100 /mod n:put $. c:put n:put ;
  :eol?    (c-f)  [ ASCII:CR eq? ] [ ASCII:LF eq? ] [ ASCII:SPACE eq? ] tri or or ;
  :valid?  (s-sf) dup s:length n:-zero? ;
  :ok      (-)    @NoEcho not 0; drop compiling? [ nl 'Ok_ s:put ] -if ;
  :check-eof (c-c) dup [ #-1 eq? ] [ #4 eq? ] bi or [ 'bye d:lookup d:xt fetch call ] if ;
  :check-bs  (c-c) dup [ #8 eq? ] [ #127 eq? ] bi or [ buffer:get buffer:get drop-pair ] if ;
  :s:get      (-s) [ #1025 buffer:set
                     [ c:get dup buffer:add check-eof check-bs eol? ] until
                      buffer:start s:chop ] buffer:preserve ;
---reveal---
  :banner  (-)    @NoEcho not 0; drop
                  'RETRO_12_(rx- s:put version $) c:put nl
                  EOM n:put '_MAX,_TIB_@_1025,_Heap_@_ s:put here n:put nl ;
  :bye     (-)    #0 unix:exit ;
  :listen  (-)
    ok repeat s:get valid? [ interpret ok ] [ drop ] choose again ;
}}
~~~

~~~
{{
  :gather (c-)
    dup [ #8 eq? ] [ #127 eq? ] bi or [ drop ] [ buffer:add ] choose ;
  :cycle (q-qc)  repeat c:get dup-pair swap call not 0; drop gather again ;
---reveal---
  :parse-until (q-s)
    [ s:empty buffer:set cycle drop-pair buffer:start ] buffer:preserve ;
}}

:s:get (-s) [ [ ASCII:LF eq? ] [ ASCII:CR eq? ] bi or ] parse-until ;
~~~

