# RETRO

This is a set of extensions for RRE.

# Console Input

~~~
:getc (-c) `1001 ;
~~~

---------------------------------------------------------------

# Floating Point

~~~
:n:to-float  (n-_f:-n)   #0 `-6000 ;
:s:to-float  (s-_f:-n)   #1 `-6000 ;
:f:to-string (f:n-__-s) s:empty dup #2 `-6000 ;
:f:+     (f:ab-c)    #3 `-6000 ;
:f:-     (f:ab-c)    #4 `-6000 ;
:f:*     (f:ab-c)    #5 `-6000 ;
:f:/     (f:ab-c)    #6 `-6000 ;
:f:floor (f:ab-c)    #7 `-6000 ;
:f:eq?   (f:ab-c)    #8 `-6000 ;
:f:-eq?  (f:ab-c)    #9 `-6000 ;
:f:lt?   (f:ab-c)   #10 `-6000 ;
:f:gt?   (f:ab-c)   #11 `-6000 ;
:f:depth (-n)       #12 `-6000 ;
:f:dup   (f:a-aa)   #13 `-6000 ;
:f:drop  (f:a-)     #14 `-6000 ;
:f:swap  (f:ab-ba)  #15 `-6000 ;
:f:log   (f:ab-c)   #16 `-6000 ;
:f:power (f:ab-c)   #17 `-6000 ;
:f:to-number (f:a-__-n)  #18 `-6000 ;
:f:sin   (f:f-f)    #19 `-6000 ;
:f:cos   (f:f-f)    #20 `-6000 ;
:f:tan   (f:f-f)    #21 `-6000 ;
:f:asin  (f:f-f)    #22 `-6000 ;
:f:acos  (f:f-f)    #23 `-6000 ;
:f:atan  (f:f-f)    #24 `-6000 ;
:f:over  (f:ab-aba) f:to-string f:dup s:to-float f:swap ;
:f:tuck  (f:ab-bab) f:swap f:over ;
:f:positive? (-f__f:a-) #0 n:to-float f:gt? ;
:f:negative? (-f__f:a-) #0 n:to-float f:lt? ;
:f:negate (f:a-b)  #-1 n:to-float f:* ;
:f:abs    (f:a-b)  f:dup f:negative? [ f:negate ] if ;
:prefix:. (s-__f:-a)
  compiling? [ s:keep ] [ s:temp ] choose &s:to-float class:word ; immediate
:putf (f:a-) f:to-string puts ;
:f:PI (f:-F) .3.141592 ;
:f:E  (f:-F) .2.718281 ;
~~~

---------------------------------------------------------------

# Scripting: Command Line Arguments

~~~
:sys:argc (-n) `-6100 ;
:sys:argv (n-s) s:empty swap `-6101 ;
~~~

---------------------------------------------------------------

# File I/O

This implements words for interfacing with the POSIX file I/O words if
you are using an interface supporting them. All of these are in the
`file:` namespace.

These are pretty much direct wrappers for fopen(), fclose(), etc.

First up, constants for the file modes.

| # | Used For           |
| - | ------------------ | 
| R | Mode for READING   |
| W | Mode for WRITING   |
| A | Mode for APPENDING |

~~~
#0 'file:R const
#1 'file:W const
#2 'file:A const
#3 'file:R+ const
~~~

For opening a file, provide the file name and mode. This will return a
number identifying the file handle.

~~~
:file:open  (sm-h) `118 ;
~~~

Given a file handle, close the file.

~~~
:file:close (h-)   `119 ;
~~~

Given a file handle, read a character.

~~~
:file:read  (h-c)  `120 ;
~~~

Write a character to an open file.

~~~
:file:write (ch-)  `121 ;
~~~

Return the current pointer within a file.

~~~
:file:tell  (h-n)  `122 ;
~~~

Move the file pointer to the specified location.

~~~
:file:seek  (nh-)  `123 ;
~~~

Return the size of the opened file.

~~~
:file:size  (h-n)  `124 ;
~~~

Given a file name, delete the file.

~~~
:file:delete (s-)  `125 ;
~~~

Flush pending writes to disk.

~~~
:file:flush (f-)   `126 ;
~~~

Given a file name, return `TRUE` if it exists or `FALSE` otherwise.

~~~
:file:exists?  (s-f)
  file:R file:open dup n:-zero?
  [ file:close TRUE ]
  [ drop FALSE ] choose ;
~~~

With that out of the way, we can begin building higher level functionality.

The first of these reads a line from the file. This is read to `here`; move
it somewhere safe if you need to keep it around.

The second goes with it. The `for-each-line` word will invoke a combinator
once for each line in a file. This makes some things trivial. E.g., a simple
'cat' implementation could be as simple as:

  'filename [ puts nl ] file:for-each-line

~~~
{{
  'FID var
  'FSize var
  'Action var
  'Buffer var
  :-eof? (-f) @FID file:tell @FSize lt? ;
  :preserve (q-) &FID [ &FSize [ call ] v:preserve ] v:preserve ;
---reveal---
  :file:read-line (f-s)
    !FID
    [ here dup !Buffer buffer:set
      [ @FID file:read dup buffer:add
        [ ASCII:CR eq? ] [ ASCII:LF eq? ] [ ASCII:NUL eq? ] tri or or ] until
        buffer:get drop ] buffer:preserve
    @Buffer ;

  :file:for-each-line (sq-)
    [ !Action
      file:R file:open !FID
      @FID file:size !FSize
      [ @FID file:read-line @Action call -eof? ] while
      @FID file:close
    ] preserve ;
}}
~~~

`file:slurp` reads a file into a buffer.

~~~
{{
  'FID var
  'Size var
---reveal---
  :file:slurp (as-)
    [ file:R file:open !FID
      buffer:set
      @FID file:size !Size
      @Size [ @FID file:read buffer:add ] times
      @FID file:close
    ] buffer:preserve ;
}}
~~~

~~~
{{
  'FID var
---reveal---
  :file:spew (ss-)
    file:W file:open !FID [ @FID file:write ] s:for-each @FID file:close ; 
}}
~~~

# Interactive Listener

~~~

{{
  :version (-)    @Version #100 /mod putn $. putc putn ;
  :eol?    (c-f)  [ ASCII:CR eq? ] [ ASCII:LF eq? ] [ ASCII:SPACE eq? ] tri or or ;
  :valid?  (s-sf) dup s:length n:-zero? ;
  :ok      (-)    compiling? [ nl 'Ok_ puts ] -if ;
  :check-eof (c-c) dup [ #-1 eq? ] [ #4 eq? ] bi or [ 'bye d:lookup d:xt fetch call ] if ;
  :check-bs  (c-c) dup [ #8 eq? ] [ #127 eq? ] bi or [ buffer:get drop ] if ;
  :gets      (-s) [ #1025 buffer:set
                    [ getc dup buffer:add check-eof check-bs eol? ] until
                    buffer:start s:chop ] buffer:preserve ;
---reveal---
  :banner  (-)    'RETRO_12_(rx- puts version $) putc nl
                  EOM putn '_MAX,_TIB_@_1025,_Heap_@_ puts here putn nl ;
  :listen  (-)
    ok repeat gets valid? [ interpret ok ] [ drop ] choose again ;
}}
~~~

~~~
:include (s-) `-9999 ;
~~~

~~~
{{
  :gather (c-)
    dup [ #8 eq? ] [ #127 eq? ] bi or [ drop ] [ buffer:add ] choose ;
  :cycle (q-qc)  repeat getc dup-pair swap call not 0; drop gather again ;
---reveal---
  :parse-until (q-s)
    [ s:empty buffer:set cycle drop-pair buffer:start ] buffer:preserve ;
}}

:gets (-s) [ [ ASCII:LF eq? ] [ ASCII:CR eq? ] bi or ] parse-until ;
~~~
