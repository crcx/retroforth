# Building RETRO

## Requirements

- c compiler (tested: clang, tcc, gcc)
- make
- standard unix shell

## Process

Run `make`.

This will build the toolchain and then the main `retro`
executable.

## Executables

In the `bin/` directory:

    retro
    retro-unu
    retro-muri
    retro-extend
    retro-embedimage

# Alternate Interfaces

The main interface, rre, is built as the `retro`
executable. But there are other options:

## retro-repl

A basic interactive system can be built by using:

    make bin/retro-repl

This requires a copy of `ngaImage` to be in the current
directory.

## retro-ri

`ri` is a full screen, curses based interface to RETRO.
It provides a display like:


    +-------------------------------+
    | ... output ...                |
    |                               |
    |                               |
    |                               |
    +-------------------------------+
    | status                        |
    +-------------------------------+
    | input                         |
    +-------------------------------+

Input is processed as it is being entered with output
displayed above. `ri` is different from the other user
interfaces as it provides a means of switching between
five separate images.

Additional Dependencies:

- curses

To build this:

    make bin/retro-ri

## Barebones

This is a minimal version of the `retro-repl`. It keeps
the C portion as short as possible, making it a useful
starting point for new interfaces.

To build:

    make bin/retro-barebones

## RETRO12.html

There is a JavaScript and HTML based interface. To
build this:

    make bin/RETRO12.html

It is tested on Chrome and Safari. The interface is
based on the UI used on the iOS and macOS versions,
presenting a dual-pane model with an editor to the
left, an output area to the right, and a listener
at the bottom.

## Pascal

There is a Pascal version of `retro-repl`.

Dependencies:

- freepascal

Building:

    cd interfaces/pascal
    fpc listener.lpr

This will require a copy of the `ngaImage` in the
current directory.

