#!/usr/bin/env retro

# A Block Editor

I've written numerous block editors over the years. This is a
new one that I'm planning to eventually use with Retro/Native.

This presents a visual, (briefly) modal interface.

              *
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
      | Output is shown here                                             |
      |                                                                  |
      |                                                                  |
      |                                                                  |
        0---------1---------2---------3---------4---------5---------6---
    #2 1:0 ::

The horizontal rulers have column indicators, there are cursor
position indicators (and the actual cursor location shows at the
intersection). The bottom line has the current block number, line
number, and column number.

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
    |  `  | Clear output buffer            |

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

There is also one half block set aside for the output display.
This is the `TOB` (Text Output Buffer).

~~~
'Block d:create #1025 allot

'Blocks d:create
  BLOCKS #1024 * allot

'TOB   d:create #513  allot
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
  :indicator  '____ s:put pad-to-cur $* c:put pad-to-eol nl ;
  :block#     $# c:put @CurrentBlock n:put ;
  :pos        @CurrentLine n:put $: c:put @CurrentCol n:put ;
  :status     block# sp pos '_::_ s:put dump-stack nl ;
  :current?   I @CurrentLine eq? ;
  :left       [ $* c:put ] &sp choose ;
  :format     current? left '_|_ s:put call '_| s:put nl ;
  :line       [ #64 [ fetch-next c:put ] times ] format ;
  :code       &Block #16 [ line ] times<with-index> drop ;
  :format     '__|_ s:put call '_| s:put nl ;
  :line       [ #64 [ fetch-next c:put ] times ] format ;
  :tob        &TOB #4 [ line ] times<with-index> drop ;
---reveal---
  :block:display (-)
    tty:clear indicator ruler code ruler tob ruler status ;
}}
~~~

# Text Output Buffer

~~~
{{
  'TNext var

  :scroll? (-f)
    @TNext #256 gt? ;
  :scroll-up (-)
    &TOB #64 + &TOB #192 copy #193 !TNext
    &TOB #193 + #64 [ #32 over store n:inc ] times drop ;
---reveal---
  :c:put<TOB> (c-)
    dup #10 eq? [ drop @TNext #64 + dup #64 mod - !TNext ]
                [ @TNext &TOB + store &TNext v:inc ] choose
    scroll? [ scroll-up ] if ;

  :with-tob (q-)
    &c:put<TOB> &c:put set-hook call &c:put unhook ;

  :tob:initialize (-)
    &TOB #512 [ #32 over store n:inc ] times drop #0 !TNext ;
}}
~~~

# The Editor Core

~~~
{{
  :cursor
    ASCII:ESC c:put
    $[ c:put @CurrentLine #3 + n:put
    $; c:put @CurrentCol #5 + n:put $H c:put ;

  :handler
    c:get
    'editor:key<_> [ #11 + store ] sip d:lookup
      dup n:-zero? [ d:xt fetch call ] [ drop ] choose ;

---reveal---

  :edit
    tob:initialize blocks:initialize block:read #0 block:select
    'stty_cbreak unix:system tty:clear
    repeat block:display cursor handler again ;
}}
~~~

# The Rest

# Key Handlers

All of the actual work is done by the words that make up
the key handlers.

## Evaluate block

The `1` key is used to evaluate a block. To avoid wasting
space, a block of memory is allocated to use as a temporary
`Heap`. The actual `Heap` is saved and restored during the
tokenization.

~~~
{{
  'Tokens d:create  #4097 allot
  :prepare  &Tokens #4096 [ #0 over store n:inc ] times drop ;
  :generate &Tokens !Heap &Block #32 s:tokenize ;
  :item     dup s:length n:zero? [ drop ] [ interpret ] choose ;
  :process  [ item ] set:for-each ;
---reveal---
  :editor:key<1>
    [ prepare &Heap [ generate ] v:preserve process ] with-tob ;
}}
~~~

## Cursor and Block Movement

~~~
{{
  :boundaries
    &CurrentLine #0 #15 v:limit
    &CurrentCol  #0 #63 v:limit ;
  :keep-in-range #0 BLOCKS n:dec n:limit ;
---reveal---
  :editor:key<H>
    block:update
    @CurrentBlock n:dec keep-in-range block:select tty:clear ;
  :editor:key<S>
    block:update
    @CurrentBlock n:inc keep-in-range block:select tty:clear ;
  :editor:key<n>  &CurrentLine v:dec boundaries ;
  :editor:key<h>  &CurrentCol  v:dec boundaries ;
  :editor:key<s>  &CurrentCol  v:inc boundaries ;
  :editor:key<t>  &CurrentLine v:inc boundaries ;
}}
~~~

## Navigate to prior, next word

These are helpful to quickly navigate through a block.

~~~
{{
  :limit       (n-n)   &Block dup #1024 + n:limit ;
  :fetch-prior (a-Ac)  [ n:dec ] [ fetch ] bi ;
  :find-next-word
    @CurrentLine #64 * @CurrentCol + &Block + n:inc
    repeat fetch-next #32 -eq? 0; drop again ;
  :find-prior-word
    @CurrentLine #64 * @CurrentCol + &Block + n:dec
    repeat fetch-prior #32 -eq? 0; drop again ;
  :select-next (-)
    find-next-word n:dec limit &Block -
    #64 /mod !CurrentLine !CurrentCol ;
  :select-prior (-)
    find-prior-word n:inc limit &Block -
    #64 /mod !CurrentLine !CurrentCol ;
---reveal---
  :editor:key<c>  select-prior ;
  :editor:key<r>  select-next ;
}}
~~~

## Edit (Replace)

~~~
{{
  :position-cursor
    ASCII:ESC c:put
    $[ c:put @CurrentLine #3 + n:put
    $; c:put @CurrentCol #5 + n:put $H c:put ;
  :dest @CurrentLine #64 * @CurrentCol + &Block + ;
  :chars over s:length ;
  :copy [ dup-pair &fetch dip store &n:inc bi@ ] times ;
---reveal---
  :editor:key<a>
    position-cursor s:get dest chars copy drop-pair tty:clear ;
}}
~~~

## Misc.

~~~
:editor:key<i>  block:update block:write ;
:editor:key<y>  @CurrentBlock block:select ;
:editor:key<x>  &Block #1024 [ #32 over store n:inc ] times drop ;
:editor:key<q>  block:write 'stty_-cbreak unix:system #0 unix:exit ;
:editor:key<`>  tob:initialize ;
~~~


# Run It

~~~
edit
~~~
