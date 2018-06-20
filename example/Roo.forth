#!/bin/sh
stty cbreak
cat >/tmp/_roo.forth << 'EOF'

# Roo: A Block Editor for RETRO

This implements a block editor for RETRO. It provides a visual interface,
inspired by VIBE and REM and allows for extending the environment by
simply writing new words that handle specific keys. It also requires a
network connection and the *Tuporo* Gopher server which provides the
actual block store.

Some architectural notes:

- the blocks are stored on a server, not locally
- editing is modal (ala *vi*)
- editor commands are just normal words in the dictionary
- this uses ANSI escape sequences and so requires a traditional terminal
  or terminal emulator



## Configuration

So getting started, some configuration settings for the server side:

~~~
:SERVER (-sn)  'forthworks.com #8008 ;
~~~

`SERVER` returns the server url and port.

Next, create a buffer to store the currently loaded block. With the
server-side storage I don't need to keep more than the current block
in memory.

A block is 1024 bytes; this includes one additional to use a terminator.
Doing this allows the block to be passed to `s:evaluate` as a string.

~~~
'Block d:create
  #1025 allot
~~~

I also define a variable, `Current-Block`, which holds the number of
the currently loaded block.

~~~
'Current-Block var
~~~


## Server Communication

With that done, it's now time for a word to load a block from the
server.

Roo requires an associated gopher server (tuporo). This is a special
server that provides access to Forth blocks across a network. The
selectors we are interested in are:

    /r/<block#>

Which returns a raw block (1024 bytes), and:

    /s/<block#>/text

Which copies the text into the specified block.

So first, define words to construct the selectors:

~~~
:selector<get>  (-s)  @Current-Block '/r/%n s:format ;
:selector<set>  (-s)  &Block @Current-Block '/s/%n/%s s:format ;
~~~

And then words to actually talk to the server:

~~~
:load-block     (-)   &Block SERVER selector<get> gopher:get drop ;
:save-block     (-)   here   SERVER selector<set> gopher:get drop ;
~~~

All done :)


## Modes

The `Mode` variable will be used to track the current mode. I have
chosen to implement two modes: command ($C) and insert ($I).

Command mode will be used for all non-entry related options, including
(but not limited to) cursor movement, block navigation, and code
evaluation.

So with two modes I only need one variable to track which mode is
active, and a single word to switch back and forth between them.

~~~
$C 'Mode var<n>
:toggle-mode (-)  @Mode $C eq? [ $I ] [ $C ] choose !Mode ;
~~~


## Cursor & Positioning

I need a way to keep track of where in the block the user currently is.
So two variables: one for the row and one for the column:

~~~
'Cursor-Row var
'Cursor-Col var
~~~

To ensure that the cursor stays within the block, I am implementing a
`constrain` word to limit the range of the cursor. Thanks to `v:limit`
this is really easy.

~~~
:constrain (-)
  &Cursor-Row #0 #15 v:limit
  &Cursor-Col #0 #63 v:limit ;
~~~

And then the words to adjust the cursor positioning:

~~~
:cursor-left   (-)  &Cursor-Col v:dec constrain ;
:cursor-right  (-)  &Cursor-Col v:inc constrain ;
:cursor-up     (-)  &Cursor-Row v:dec constrain ;
:cursor-down   (-)  &Cursor-Row v:inc constrain ;
~~~

The other bit related to the cursor is a word to decide the offset into
the block. This will be used to aid in entering text.

~~~
:cursor-position  (-n)  @Cursor-Row #64 * @Cursor-Col + ;
~~~

The last bit here is `insert-character` which inserts a character to
`cursor-position` in the `Block` and moves the cursor to the right.

~~~
:insert-character (c-) cursor-position &Block + store cursor-right ;
~~~


## Keyboard Handling

Handling of keys is essential to using Roo. I chose to use a method that
I borrowed from Sam Falvo II's VIBE editor and leverage the dictionary
for key handlers.

In Roo a key handler is a word in the `roo:` namespace. A word like:

    roo:c:a

Will implement a handler called when 'a' is typed in command mode. And

    roo:i:`

Would implement a handler for the '`' key in insert mode.

In command mode keys not matching a handler are ignored. For words that
do match up to a control word, the word will be called. In insert mode,
any keys not mapped to a word will be inserted into the block at the
current position.

My default keymap will be (subject to change!):

    `    Switch modes
    h    Cursor left
    j    Cursor down
    k    Cursor up
    l    Cursor right
    H    Previous block
    L    Next block
    e    Evaluate block
    q    Quit

Getting started, I define a word to take a character and pack it into a
string. It then tries to find this in the dictionary.

~~~
:handler-for (c-d)
  @Mode $C eq? [ 'roo:c:_ ]
               [ 'roo:i:_ ] choose [ #6 + store ] sip d:lookup ;
~~~

With that, I can implement another helper: `call<dt>`, which will take
the dictionary token returned by `handler-for` and call the xt for the
word.

~~~
:call<dt>  (d-)  d:xt fetch call ;
~~~

The final piece is the top level key handler. This has the following
jobs:

- try to find a handler for the key
  - if mode is $C and the handler is valid, call the handler
  - if mode is $I and the handler is invalid, insert the key into the
    block
  - if mode is $I and the handler is valid, call the handler

~~~
:handle-key (c-)
  dup handler-for
  @Mode $I -eq? [ nip 0; call<dt> ]
                [ dup n:zero? [ drop insert-character ]
                              [ nip call<dt>          ] choose ] choose ;
~~~

Having finished this, it's trivial to define the majority of the basic
commands:

~~~
:roo:c:H &Current-Block v:dec load-block ;
:roo:c:L &Current-Block v:inc load-block ;
:roo:c:h cursor-left ;
:roo:c:j cursor-down ;
:roo:c:k cursor-up ;
:roo:c:l cursor-right ;
:roo:c:e &Block s:evaluate ;
:roo:c:` toggle-mode ;
~~~

I only define one command in input mode, to switch back to command mode:

~~~
:roo:i:` toggle-mode save-block ;
~~~

Note that this calls `save-block` to update the remote block storage.
This is the only place I call `save-block`.

One last word is a handler to allow the editor to be closed cleanly.
This also has a variable, `Completed`, which will be used to decide
if editing is finished.

~~~
'Completed var
:roo:c:q &Completed v:on ;
~~~


## Display

The block display is kept minimalistic. Each line is bounded by a single
vertical bar (|) on the right edge, and there is a separatator line at
the bottom to indicate the base of the block. To the left of this is a
single number, indicating the current block number. This is followed by
the mode indicator.

I also display the current stack contents below the block.

The display looks like:

    (blank)                                                         |
    :roo:c:+ (nn-m) + ;                                             |
    :roo:c:1 (-n)  #1 ;    :roo:c:2 (-n)  #2 ;                      |
    :roo:c:4 (-n)  #4 ;    :roo:c:3 (-n)  #3 ;                      |
                                                                    |
                                                                    |
                                                                    |
                                                                    |
                                                                    |
                                                                    |
                                                                    |
                                                                    |
                                                                    |
                                                                    |
                                                                    |
                                                                    |
    ----------------------------------------------------------------+ 29C
    1 2 <3>

The cursor display will be platform specific.

~~~
:position-cursor (-)
  @Cursor-Col @Cursor-Row [ n:inc ] bi@
  ASCII:ESC '%c[%n;%nH s:format s:put ;

:clear-display (-)
  ASCII:ESC c:put '[2J s:put
  ASCII:ESC c:put '[H s:put ;

:display-block (-)
  clear-display
  &Block #16 [ #64 [ fetch-next c:put ] times $| c:put nl ] times drop
  #64 [ $- c:put ] times $+ c:put sp @Current-Block n:put @Mode c:put nl
  dump-stack position-cursor ;
~~~


## The Final Piece

All that's left is a single top level loop to tie it all together.

~~~
:edit
  &Completed v:off
  #0 !Current-Block load-block
  [ display-block c:get handle-key @Completed ] until ;

edit
~~~



EOF
rre /tmp/_roo.forth
rm -f /tmp/_roo.forth
stty -cbreak

