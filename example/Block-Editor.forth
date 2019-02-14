#!/usr/bin/env retro

# A Block Editor

I've written numerous block editors over the years. This is a
new one that I'm planning to eventually use with Retro/Native.

This presents a visual, (briefly) modal interface.

        0---------1---------2---------3---------4---------5---------6---
      | This is the new block editor!                                    |
    * |                                                                  |
      |                                                                  |
      |                                                                  |
      |                                                                  |
      |                                                                  |
      |                                                                  |
      |                                                                  |
      |                                                                  |
      |                                                                  |
      |                                                                  |
      |                                                                  |
      |                                                                  |
      |                                                                  |
      |                                                                  |
      |                                                                  |
        0---------1---------2---------3---------4---------5---------6---
        *                                                                #2 1:0

The horizontal rulers have column indicators, there are cursor
position indicators (and the actual cursor location shows at the
intersection). The bottom right has the current block number,
line number, and column number.

Key Bindings

    | Key |  Action                        |
    | --- | ------------------------------ |
    |  H  | Save and select previous block |
    |  S  | Save and select next block     |
    |  h  | Move cursor left               |
    |  t  | Move cursor down               |
    |  n  | Move cursor up                 |
    |  s  | Move cursor right              |
    |  i  | Save the current block         |
    |  y  | Reload the current block       |
    |  a  | Edit text                      |
    |  c  | Move cursor to prior word      |
    |  r  | Move cursor to next word       |
    |  1  | Evaluate the current block     |
    |  x  | Erase the current block        |
    |  q  | Save the Blocks and Quit       |

The key bindings are oriented around the Dvorak keyboard layout
which I use. The key map leverages an approach I stole from
Samuel Falvo II's VIBE editor: the key handlers are words in
the dictionary with a format like:

    editor:key<*>

With the `*` being the key.

# Configuration

I define the number of blocks and the file to use. On the non
hosted Retro systems the block file will be replaced by a
block storage device.

~~~
#512          'BLOCKS       const
'retro.blocks 'BLOCK-FILE s:const
~~~

# Variables

I am keeping some data in variables. This will include the
current block number, the current line, and the current row.

~~~
'CurrentBlock var
'CurrentLine  var
'CurrentCol   var
~~~

# Buffers

I have one memory region for the current block and a second
one for the entire set of blocks. I keep the blocks in RAM
for performance.

~~~
'Block d:create #1025 allot

'Blocks d:create
  BLOCKS #1024 * allot
~~~

# Block File I/O

This should be replaced with the block storage device once
that is added to Nga.

~~~
:blocks:initialize (-)
  &Blocks #1024 BLOCKS * [ #32 over store n:inc ] times drop ;

:block:write (-)
  &Blocks BLOCK-FILE file:open<for-writing>
  BLOCKS #1024 * [ over fetch over file:write &n:inc dip ] times
  file:close drop ;

:block:read (-)
  BLOCK-FILE file:exists? [ block:write ] -if
  &Blocks BLOCK-FILE file:open<for-reading> swap
  [ dup-pair file:read swap store &n:inc dip ] times
  file:close drop ;
~~~

These are used to load a block into the active buffer and
copy it back to the full set.

~~~
{{
  :current-block @CurrentBlock #1024 * &Blocks + ;
  :next          [ n:inc ] bi@ ;
  :copy          dup-pair fetch swap store ;
---reveal---
  :block:select (n-)
    !CurrentBlock
    [ &Block buffer:set
      current-block #1024
      [ fetch-next buffer:add ] times drop ] buffer:preserve ;

  :block:update (-)
    current-block &Block #1024 [ copy next ] times drop-pair ;
}}
~~~

# Block Display

This is fairly long, but mostly due to my desire to have a
more complex display with column and cursor indicators.
It should be pretty straightforward though.

~~~
:tty:clear (-) #27 c:put '[2J s:put #27 c:put '[0;0H s:put ;

{{
  :ruler 
    '____0---------1---------2---------3 s:put
    '---------4---------5---------6---   s:put nl ;
  :pad-to-cur @CurrentCol dup [ sp ] times ;
  :pad-to-eol #64 swap - [ sp ] times ;
  :indicator  '____ s:put pad-to-cur $* c:put pad-to-eol ;
  :block#     $# c:put @CurrentBlock n:put ;
  :pos        @CurrentLine n:put $: c:put @CurrentCol n:put ;
  :status     block# sp pos nl dump-stack nl ;
  :current?   I @CurrentLine eq? ;
  :left       [ $* c:put ] &sp choose ;
  :format     current? left '_|_ s:put call '_| s:put nl ;
  :line       [ #64 [ fetch-next c:put ] times ] format ;
  :code       &Block #16 [ line ] times<with-index> drop ;
---reveal---
  :block:display (-)
    tty:clear ruler code ruler indicator status ;
}}
~~~

# The Editor Core

~~~
{{
  :cursor
    ASCII:ESC c:put
    $[ c:put @CurrentLine #2 + n:put
    $; c:put @CurrentCol #5 + n:put $H c:put ;

  :handler
    c:get
    'editor:key<_> [ #11 + store ] sip d:lookup
      dup n:-zero? [ d:xt fetch call ] [ drop ] choose ;

---reveal---

  :edit
    blocks:initialize block:read #0 block:select
    'stty_cbreak unix:system tty:clear
    repeat block:display cursor handler again ;
}}
~~~

# The Rest

# Key Handlers

All of the actual work is done by the words that make up
the key handlers.

~~~
{{
  'Tokens d:create  #4097 allot
---reveal---
  :editor:key<1>
    &Tokens #4096 [ #0 over store n:inc ] times drop
    &Heap [ &Tokens !Heap &Block #32 s:tokenize ] v:preserve
    [ dup s:length n:zero? [ drop ] [ interpret ] choose ] set:for-each ;
}}

:editor:key<H>  block:update @CurrentBlock n:dec #0 BLOCKS n:dec n:limit block:select tty:clear ;
:editor:key<S>  block:update @CurrentBlock n:inc #0 BLOCKS n:dec #18 &nl times dump-stack n:limit block:select tty:clear ;
:editor:key<n>  &CurrentLine v:dec &CurrentLine  #0 #15 v:limit ;
:editor:key<h>  &CurrentCol  v:dec &CurrentCol   #0 #63 v:limit ;
:editor:key<s>  &CurrentCol  v:inc &CurrentCol   #0 #63 v:limit ;
:editor:key<t>  &CurrentLine v:inc &CurrentLine  #0 #15 v:limit ;

{{
  :cursor
    ASCII:ESC c:put
    $[ c:put @CurrentLine #2 + n:put
    $; c:put @CurrentCol #5 + n:put $H c:put ;
---reveal---
  :editor:key<a>  cursor s:get @CurrentLine #64 * @CurrentCol + &Block + over s:length [ dup-pair &fetch dip store &n:inc bi@ ] times drop-pair #27 c:put '[2J s:put ;
}}

:editor:key<i>  block:update block:write ;
:editor:key<y>  @CurrentBlock block:select ;
:editor:key<x>  &Block #1024 [ #32 over store n:inc ] times drop ;
:editor:key<q>  block:write 'stty_-cbreak unix:system #0 unix:exit ;
~~~

~~~
{{
  :limit       (n-n)   &Block dup #1024 + n:limit ;

  :fetch-prior (a-Ac)  [ n:dec ] [ fetch ] bi ;

  :find-next-word
    @CurrentLine #63 * @CurrentCol + &Block + n:inc
    repeat fetch-next #32 -eq? 0; drop again ;

  :find-prior-word
    @CurrentLine #63 * @CurrentCol + &Block + n:dec
    repeat fetch-prior #32 -eq? 0; drop again ;

  :select-next (-)
    find-next-word n:dec limit &Block -
    #63 /mod !CurrentLine !CurrentCol ;

  :select-prior (-)
    find-prior-word n:inc limit &Block -
    #63 /mod !CurrentLine !CurrentCol ;

---reveal---

  :editor:key<c>  select-prior ;

  :editor:key<r>  select-next ;
}}
~~~


# Run It

~~~
edit
~~~
