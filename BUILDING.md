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

## retro-compiler

This is a turnkey compiler. It can compile a new executable
bundling a Retro VM and image.

Requirements:

- BSD or Linux
- objcopy in $PATH

To build:

    make bin/retro-compiler

Example use:

1. Given a source file like "Hello.forth":

    ~~~
    :hello 'hello_world! s:put nl ;
    ~~~

2. Use:

    ./bin/retro-compiler Hello.forth hello

The first argument is the source file, the second is the
word to run on startup.

3. Run the generated `a.out`

Limits:

This only supports the core words ('all' interface) and the
file i/o words. Support for other I/O extensions will be
added in the future.

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

## Python: retro.py

This is an implementation of `retro-repl` in Python. As
with `retro-repl` it requires the `ngaImage` in the current
directory when starting.

## C#: retro.cs

This is an implementation of `retro-repl` in C#. As with
`retro-repl` it requires the `ngaImage` in the current
directory when starting.

Building:

    csc retro.cs

You'll need to make sure your path has the CSC.EXE in it,
or provide a full path to it. Something like this should
reveal the path to use:

    dir /s %WINDIR%\CSC.EXE

I've only tested building this using Microsoft's .NET tools.
It should also build and run under Mono.
