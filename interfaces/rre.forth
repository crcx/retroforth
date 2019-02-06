# RRE Listener and Extensions

In this file I am implementing the interactive listener that
RRE will run when started with `-i` or `-c`.

## Console Input

The RRE interface provides a keyboard device. This exposes it
via `c:get`.

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

Now that I can read characters, it's time to support reading
strings. I do this via two words. The first is `parse-until`.
This will setup a temporary string as an input buffer, then
read input, passing each character ot a provided quote. When
the quote returns `TRUE`, it ends and returns the string. When
not `TRUE` it will add the character to the buffer.

~~~
{{
  :gather (c-)
    dup [ #8 eq? ] [ #127 eq? ] bi or [ drop ] [ buffer:add ] choose ;
  :cycle (q-qc)  repeat c:get dup-pair swap call not 0; drop gather again ;
---reveal---
  :parse-until (q-s)
    [ s:empty buffer:set cycle drop-pair buffer:start ] buffer:preserve ;
}}
~~~

Using this, a simple `s:get` can be implemented very easily as
a quote which looks for an end of line character.

~~~
:s:get (-s) [ [ ASCII:LF eq? ] [ ASCII:CR eq? ] bi or ] parse-until ;
~~~


## Scripting: Command Line Arguments

RRE also provides access to the command line arguments passed
to a script. The next few words map the scripting device to
words we can use.

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


## Interactive Listener

The main part of this file is the *listener*, an interactive
read-eval-print loop. 

RRE's C part will access a couple parts of this, based on the
startup flags passed.

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

&listen #1 store
~~~


~~~
:image:save (s-) #1000 io:scan-for io:invoke ;
~~~
