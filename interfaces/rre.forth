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
  :include  (s-)               identify #2 @io:Scripting io:invoke ;
  :sys:name (-s)       s:empty identify #3 @io:Scripting io:invoke ;
}}
~~~

# Fullscreen Interactive Listener

~~~
'FullScreenListener var
~~~

The basic image has a space allocated for input at the end of
the kernel. This is at address 1024 (the kernel space is fixed
at addresses 0 to 1023).

~~~
#1024 'TIB const
~~~

Nearly all of this will be hidden from the user.

~~~
{{
~~~

I'm using the buffer: words to deal with the TIB. To avoid any
conflict with user buffers, I'm duplicating the needed words
here, privately.

~~~
  :Buffer `0 ; data
  :Ptr    `0 ; data
  :terminate (-) #0 @Ptr store ;
  :buffer:start  (-a) @Buffer ;
  :buffer:end    (-a) @Ptr ;
  :buffer:add    (c-) buffer:end store &Ptr v:inc terminate ;
  :buffer:get    (-c) &Ptr v:dec buffer:end fetch terminate ;
  :buffer:empty  (-)  buffer:start !Ptr terminate ;
  :buffer:set    (a-) !Buffer buffer:empty ;
~~~

Color choices are a personal thing. I define the ones I want
here.

~~~
  :white,blue  #27 c:put '[1;37;44m s:put ;
  :white,black #27 c:put '[0;37;40m s:put ;
~~~

The top bar displays:

- stack depth
- memory stats
- top five stack values

~~~
  :dump-stack
    depth 0; #5 n:min
    '|_ s:put
    #1 [ 'a     'aa         reorder      n:put            ] case
    #2 [ 'ab    'abba       reorder #2 [ n:put sp ] times ] case
    #3 [ 'abc   'abccba     reorder #3 [ n:put sp ] times ] case
    #4 [ 'abcd  'abcddcba   reorder #4 [ n:put sp ] times ] case
    #5 [ 'abcde 'abcdeedcba reorder #5 [ n:put sp ] times ] case ;

  :top-row    #27 c:put '[1;0H s:put ;
  :clear-row  #80 [ sp ] times ;

  :top   top-row white,blue clear-row top-row
         FREE depth n:dec 'SP:%n_FREE:%n_ s:format s:put
         dump-stack ;
~~~


Next is the input bar. This is displayed at the bottom of the screen.

~~~
  :bottom-row  #27 c:put '[24;0H s:put ;
  :input       bottom-row white,blue clear-row bottom-row ;
~~~

The other bit is the output area in the middle. I have this as white
text on a black background.

~~~
  :output bottom-row white,black clear-row bottom-row ;
~~~

Now on to input processing. I have a variable, `In`, which
tracks the number of characters read so far. This is used in
a few places, to ensure that backspace doesn't overwrite the
kernel, and to blank the line if too many characters are
input.

~~~
  'In var
~~~

For handling backspaces, remove from the buffer and erase the
displayed character.

~~~
  :discard buffer:get drop ;
  :rub     #8 c:put sp #8 c:put ;
~~~

I process input on CR, LF, and SPACE.

~~~
  :evaluate TIB interpret ;
  :reset    #0 !In buffer:empty ;
~~~

Tie this together into a key handler.

~~~
  :handle
    ASCII:SPACE [ @In 0; drop output evaluate nl reset top input ] case
    ASCII:CR    [ @In 0; drop output evaluate nl reset top input ] case
    ASCII:LF    [ @In 0; drop output evaluate nl reset top input ] case
    ASCII:DEL   [ @In 0; drop discard rub &In v:dec ] case
    ASCII:BS    [ @In 0; drop discard rub &In v:dec ] case
    dup c:put buffer:add &In v:inc ;
~~~

And use that a the `process` loop.

~~~
  :process
    TIB buffer:set
    repeat c:get handle @In #80 eq? [ #0 !In input ] if again ;
~~~

The last few support words use the stty(1) program to setup and
reset the terminal.

~~~
  :init   'stty_cbreak_-echo unix:system ;
  :exit   'stty_-cbreak_echo unix:system #0 unix:exit ;
~~~

Switch to the public words.

~~~
---reveal---
~~~

And expose `bye`, `clear`, and the `listener`.

~~~
  :clear #27 c:put '[2J s:put #27 c:put '[0;0H s:put ;
  :listen:fullscreen:bye  exit ;
  :listen:fullscreen      init clear top input process ;
~~~

Hide the support words.

~~~
}}
~~~

# Standard Interactive Listener

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
  :bye     (-)    @FullScreenListener [ listen:fullscreen:bye ] if
                  #0 unix:exit ;
  :listen  (-)
    @FullScreenListener [ listen:fullscreen ] if;
    ok repeat s:get valid? [ interpret ok ] [ drop ] choose again ;
}}

&listen #1 store
~~~


~~~
:image:save (s-) #1000 io:scan-for io:invoke ;
~~~
