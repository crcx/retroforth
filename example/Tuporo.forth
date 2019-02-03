#!/usr/bin/env retro

# tūporo: a gopher-based block store for retro

## Overview

Traditional Forths often provided simple editors oriented around blocks.
A standard block is a single unit of text, with 16 lines of 64 symbols
per line, or 1024 characters. While far less common now, blocks are
still useful, and RETRO has had some support for them in most of its
incarnations.

With my recent interest in Gopher, I've decided that my next take on
blocks will use Gopher.

So the basic goal of this is to provide a Gopher server capable of
transfer and update of blocks, and also a means of browsing the blocks
via Gopher. As with all of my recent servers, this will run under `tui`,
`tcpserver` or `inetd`.

## Configuration

The basic configuration settings are the number of blocks (`MAX-BLOCKS`)
and the path to the block file (including the file name). This is also
where the server URL (or IP) and port are set.

~~~
:BLOCKS '/home/crc/blocks ;
:SERVER 'forthworks.com ;
#8081   'PORT       const
#200    'MAX-BLOCKS const
~~~

## Block I/O

First up are a scratch variable (`FID`) to hold the file ID for use with
reads/writes, and a safe buffer to store the currently loaded block. I
will use the `buffer:` namespace for interacting with the block, so it
needs to be one cell longer than the actual data length to account for
the final ASCII NUL terminator.

~~~
'FID var
'Block d:create #1025 allot
~~~

`block:locate` moves the index in the blockfile to the actual starting
point for a particular block.

~~~
:block:locate (n-)
  #1024 * @FID file:seek ;
~~~

`block:copy` copies the data for the current block into the `Block`
buffer.

~~~
:block:copy (-)
  #1024 [ @FID file:read buffer:add ] times ;
~~~

The top level `block:get` word sets the current buffer to `Block`, then
loads the block file and copies the requested block into the buffer. It
returns the address of the `Block` buffer.

~~~
:block:get (n-s)
  &Block buffer:set
  BLOCKS file:R file:open !FID
  block:locate block:copy
  @FID file:close &Block ;
~~~

`block:set` writes a string into a block. The string *can* be longer
than a block, in which case it writes to subsequent blocks.

~~~
:block:set (sn-)
  BLOCKS file:R+ file:open !FID
  block:locate dup s:length [ fetch-next @FID file:write ] times drop
  @FID file:close ;
~~~

## Browsing

To be able to browse the blocks, we first need a means of displaying a
top level index (returned when the Gopher client sends a request as an
empty selector string).

I'll use `generate-index` for this. A Gopher directory line looks like:

    <type><description>\t<selector>\t<server>\t<port>

The type of interest here is:

    0  plain text

I define `generate-entry` to make a line for a block. It takes a
description and selector and uses the SERVER and PORT variables to
construct the line.

~~~
:generate-entry (ss-)
  SERVER PORT 'abcd 'dcba reorder '0%s\t%s\t%s\t%n s:format s:put nl ;
~~~

With this it's easy to define `generate-index` using a loop to make a
usable directory index listing all blocks.

~~~
:generate-index (-)
  #0 MAX-BLOCKS
  [ dup n:to-string over '/%n s:format generate-entry n:inc ] times
  drop ;
~~~

Displaying a block as plain text is very easy. Using `block:get` to fetch
the data, it's just two loops (one for each line, one for each charaacter)
displaying the characters and newlines as needed.

~~~
:display-block (n-)
  block:get #16 [ #64 [ fetch-next c:put ] times nl ] times drop ;
~~~

## ...

## Gopher Protocol

Tūporo decides what to do based on the selectors passed to it. These are
what I will recognize:

    /                directory index of all blocks
    /nnnn            block #nnnn (as formatted text data)
    /r/nnnn          block #nnnn (as raw text data)
    /s/nnnn/text     change block #nnnn to specified raw text data

I have a `Selector` buffer for storing the selector the user passes in.
This is sized to be big enough for the incoming block data (if using /s)
with room to spare.

~~~
'Selector d:create #4096 allot
~~~

The `prefix` word returns the first two characters of the selector. This
will be enough to identify what type of request we are dealing with.

~~~
:prefix (-s)
  &Selector #0 #2 s:substr ;
~~~

`raw-block` returns a raw, unformatted block as text data. This will
correspond to /r/nnnn selectors.

~~~
:raw-block (-)
  &Selector #3 + s:chop block:get s:put ;
~~~

`set-block` updates a block with new text. This selector takes a form:

    /s/block#/text

It's probably *not* a good idea to leave this exposed on a public
server as there is no means provided of restricting writes using it.

~~~
:set-block (-)
  &Selector #3 +
  $/ s:split s:to-number swap n:inc s:chop swap block:set ;
~~~

And `handle-block` uses `display-block` to return a formatted text block
when browsing.

~~~
:handle-block (-)
  &Selector n:inc s:chop s:to-number display-block ;
~~~

The top level `handle` word decides how to handle each selector using the
results of `prefix`. Selectors that don't match up to one of the handled
ones just return a directory listing.

~~~
:handle
  prefix
  '/r [ raw-block ] s:case 
  '/s [ set-block ] s:case
  '/0 [ handle-block ] s:case
  '/1 [ handle-block ] s:case
  '/2 [ handle-block ] s:case
  '/3 [ handle-block ] s:case
  '/4 [ handle-block ] s:case
  '/5 [ handle-block ] s:case
  '/6 [ handle-block ] s:case
  '/7 [ handle-block ] s:case
  '/8 [ handle-block ] s:case
  '/9 [ handle-block ] s:case
  drop generate-index ;
~~~

And finally, a quick bit from Atua to read in the selector and pass it
to `handle`

~~~
:eol? (c-f)
  [ ASCII:CR eq? ] [ ASCII:LF eq? ] [ ASCII:HT eq? ] tri or or ;
:s:get (a-)
  buffer:set
  [ s:get dup buffer:add eol? not ] while ;

&Selector s:get handle
~~~

## Future Direction

It'd probably be a good idea to add some authentication so unknown users
can't write changes to the block store.

Other than that, it's a simple, clean system for exposing a blockfile via
Gopher.

## Legalities

Copyright (c) 2017, Charles Childers

Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted, provided that the
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.


