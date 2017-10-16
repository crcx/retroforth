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
