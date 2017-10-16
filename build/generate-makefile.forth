#!/usr/bin/env rre

This is a tool to help in building RETRO. It will scan a source
file (in C), using comments in the file as directives to help it
in this process.

Currently this aims to support:

  //HEAD
  //FLAG
  //USES
  //LIBS

Each of these will be followed by one or more whitespace delimited
tokens. These will be broken apart and added to different lists if
not already present.

So a case:

    #include <stdio.h>
    //LIBS m curses
    //USES nga bridge image
    //FLAG -DPOSIX_FILES -DPOSIX_GETC -DPOSIX_ARGS
    //FLAG -O3
    ....

This tool will generate a Makefile suitable for compiling the
source and its dependencies:

    CC = clang
    FLAGS = -DPOSIX_FILES -DPOSIX_GETC -DPOSIX_ARGS -O3
    example:
    	$(CC) $(FLAGS) -c example.c -o example.o
    	$(CC) $(FLAGS) -c nga.c -o nga.o
    	$(CC) $(FLAGS) -c bridge.c -o bridge.o
    	$(CC) $(FLAGS) -c image.c -o image.o
    	$(CC) $(FLAGS) example.o nga.o bridge.o image.o -o example
    	rm example.o nga.o bridge.o image.o

The ultimate intention is to allow this to generate the Makefiles
for each interface automatically so that only the image rebuild
will need a manually written Makefile.

Begin by creating arrays for each type of directive.

~~~
'Uses d:create   #1024 allot
'Flag d:create   #1024 allot
'Libs d:create   #1024 allot
~~~

Next up, some extensions to the `file:` namespace. The first new
word will read a line from the file and the second one uses the
first, calling a combinator after reading each line.

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

With the `file:for-each-line`, it's easy to build something like
the Unix "cat" utility:

    #!/usr/bin/env rre
    #0 sys:argv [ puts nl ] file:for-each-line

So moving on, it's now time to begin the parser.

Our first word here is `split`, which will divide a string into
two parts. We check the second one (the first six characters)
against the commands we know how to deal with.

~~~
:split (s-ss)
  [ #7 + ] [ #0 #6 s:substr ] bi ;
~~~

This is an addition to the `set:` namespace. Sets made using
`set:from-results` and similar are fixed size with no padding. The
sets here are 1024 cells each, so new items can be appended. This
word makes it easy to do so.

~~~
:set:append (na-)
  dup v:inc dup fetch + store ;
~~~

And now for code to append unique items to each set.

TODO: refactor this - there's a lot of duplication.

~~~
:build:uses (s-)
  dup &Uses set:contains-string?
  [ drop ]
  [ dup '#SRC:_ puts puts nl s:keep &Uses set:append ] choose ;

:build:libs (s-)
  dup &Libs set:contains-string?
  [ drop ]
  [ dup '#LIB:_ puts puts nl s:keep &Libs set:append ] choose ;

:build:flag (s-)
  dup &Flag set:contains-string?
  [ drop ]
  [ dup '#FLG:_ puts puts nl s:keep &Flag set:append ] choose ;
~~~

Next up is the scanner. This is currently as simple as possible:

- split the line
- see if it starts with a directive

  - yes? process the directive
  - no? discard the line
- repeat until done

~~~
:scan (s-)
  [ split
    '//USES [ dup build:uses '.c s:append scan ] s:case
    '//HEAD [ '.h s:append scan ] s:case
    '//LIBS [ build:libs ] s:case
    '//FLAG [ build:flag ] s:case
    drop-pair
  ] file:for-each-line ;
~~~

Getting near the end, the remaining words are needed to create the
Makefile. These are short, and each is intended to handle a simple
role.

~~~
:target (-s) #0 sys:argv s:to-upper ;

:libraries
  target puts 'LIBS_=_ puts
  &Libs [ '-l puts puts sp ] set:for-each nl ;

:flags
  target puts 'FLAGS_=_ puts
  &Flag [ puts sp ] set:for-each nl ;

:build-target (-)
  #0 sys:argv puts $: putc nl ;

:compile-dependency (s-)
  target over '\tclang_-c_%s.c_$(%sFLAGS)_-o_%s.o\n s:with-format puts ;

:dependencies (-)
  &Uses [ compile-dependency ] set:for-each ;

:object-files (-)
  target '\tclang_$(%sLIBS)_ s:with-format puts
  &Uses [ puts '.o_ puts ] set:for-each
  #0 sys:argv '_-o_%s\n s:with-format puts ;

:binary (-)
 tab 'mv_ puts #0 sys:argv puts '_../bin puts nl ;
~~~

The top level "display-Makefile" is simple: all it has to do is list the
items we want in the proper order.

~~~
:display-Makefile (-)
  (declare libraries
  (declare flags
  (specify build-target
  (list    dependencies
  (link    object-files
  (move    binary
;
~~~

~~~
#0 sys:argv dup build:uses '.c s:append scan
display-Makefile
~~~
