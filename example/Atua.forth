#!/usr/bin/rre

# Atua: A Gopher Server

Atua is a gopher server written in Retro.

This will get run as an inetd service, which keeps things simple as it
prevents needing to handle socket I/O directly.

# Features

Atua is a minimal server targetting the the basic Gopher0 protocol. It
supports a minimal gophermap format for indexes and transfer of static
content.

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

# Script Prelude

Atua uses Retro's *rre* interface layer. Designed to run a single
program then exit, this makes using Retro as a scripting language
possible.

# Configuration

Atua needs to know:

- the path to the files to serve
- the name of the index file
- The maximum length of a selector
- The maximum file size

~~~
'/home/crc/atua s:keep 'PATH   const
'/gophermap     s:keep 'DEFAULT-INDEX const
#1024  'MAX-SELECTOR-LENGTH    const
'forthworks.com s:keep 'SERVER const
'70             s:keep 'PORT   const
~~~

# I/O Words

Retro only supports basic output by default. The RRE interface that
Atua uses adds support for files and stdin, so we use these and
provide some other helpers.

*Console Output*

The Gopher protocol uses tabs and cr/lf for signficant things. To aid
in this, I define output words for tabs and end of line.

~~~
:eol  (-)  ASCII:CR c:put ASCII:LF c:put ;
~~~

*Console Input*

Input lines end with a cr, lf, or tab. The `eol?` checks for this.
The `gets` word could easily be made more generic in terms of what
it checks for. This suffices for a Gopher server though.

~~~
:eol? (c-f)
  [ ASCII:CR eq? ] [ ASCII:LF eq? ] [ ASCII:HT eq? ] tri or or ;
:s:get (a-)
  buffer:set
  [ c:get dup buffer:add eol? not ] while ;
~~~

# Gopher Namespace

Atua uses a `gopher:` namespace to group the server related words.

~~~
{{
~~~

First up are buffers for the selector string and the file buffer. The
variables and buffers are kept private.

~~~
'Selector d:create
  MAX-SELECTOR-LENGTH n:inc allot

:buffer here ;
~~~

Next up, variables to track information related to the requested
selector. Atua will construct filenames based on these.

~~~
'Requested-File var
'Requested-Index var
~~~

`FID`, the file id, tracks the open file handle that Atua uses
when reading in a file. The `Size` variable will hold the size of
the file (in bytes).

~~~
'FID var
'Size var
'Mode var
~~~

I use a `Server-Info` variable to decide whether or not to display
the index footer. This will become a configurable option in the
future.

~~~
'Server-Info var
~~~

~~~
:with-path (-s)
  PATH &Selector s:chop s:append ;

:construct-filenames (-)
  with-path s:keep !Requested-File
  with-path '/gophermap s:append s:keep !Requested-Index
;
~~~

A *gophermap* is a file that makes it easier to handle Gopher menus.
Atua's gophermap support covers:

- comment lines

  Comment lines are static text without any tabs. They will be
  reformatted according to protocol and sent.

- selector lines

  Any line with a tab is treated as a selector line and is transferred
  without changing.


~~~
'Tab var
:eol? [ ASCII:LF eq? ] [ ASCII:CR eq? ] bi or ;
:tab? @Tab ;
:check-tab
  dup ASCII:HT eq? [ &Tab v:on ] if ;
:gopher:gets (a-)
  &Tab v:off
  buffer:set
  [ @FID file:read dup buffer:add check-tab eol? not ] while
  buffer:get drop
;
~~~

The internal helpers are now defined, so switch to the part of the
namespace that'll be left exposed to the world.

~~~
---reveal---
~~~

An information line s:get a format like:

  i...text...<tab><tab>null.host<tab>port<cr,lf>

The `gopher:i` displays a string in this format. It's used later for
the index footer.

~~~
:gopher:i (s-)
  'i%s\t\tnull.host\t1 s:format s:put eol ;
~~~

~~~
:gopher:get-selector (-)
  &Selector s:get ;

:gopher:file-requested? (-f)
  &Selector s:chop s:length n:-zero? ;

:gopher:valid-file? (-f)
  [ @Requested-File file:R file:open 
    file:size n:strictly-positive? ]
  [ FALSE ] choose ;

:gopher:get-index (-s)
  @Requested-Index file:exists?
  [ @Requested-Index &Server-Info v:on  ]
  [ PATH '/empty.index s:append         ] choose ;

:gopher:file-for-request (-s)
  &Mode v:off
  construct-filenames
  gopher:file-requested?
  [ @Requested-File file:exists? gopher:valid-file?
    [ @Requested-File  ]
    [ gopher:get-index ] choose
  ]
  [ PATH DEFAULT-INDEX s:append &Server-Info v:on ] choose
;

:gopher:read-file (f-s)
  file:R file:open !FID
  @FID file:size !Size
  @Size [ @FID file:read c:put ] times
  @FID file:close
;

:gopher:line (s-)
  dup [ ASCII:HT eq? ] s:filter s:length #1 gt?
  [ s:put ]
  [ s:put tab SERVER s:put tab PORT s:put ] choose
  eol ;

:gopher:generate-index (f-)
  file:R file:open !FID
  @FID file:size !Size
  [ buffer gopher:gets
    buffer tab? [ gopher:line ] [ gopher:i ] choose
    @FID file:tell @Size lt? ] while
  @FID file:close
;
~~~

The only thing left is the top level server.

~~~
:gopher:server
  gopher:get-selector
  gopher:file-for-request
  @Server-Info
  [ gopher:generate-index
    '------------------------------------------- gopher:i
    'forthworks.com:70_/_atua_/_running_on_retro gopher:i 
    '. s:put eol ]
  [ gopher:read-file ] choose
;
~~~

Close off the helper portion of the namespace.

~~~
}}
~~~

And run the `gopher:server`.

~~~
gopher:server
reset
~~~

