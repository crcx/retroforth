## RETRO FORTH

RETRO is a modern, pragmatic Forth drawing influences from many
sources. It's clean, elegant, tiny, and easy to grasp and adapt
to various uses.

Features:

- Open Source (ISC License)
- Portable (runs on a MISC-style virtual machine)
- Small source & binaries
- Builds into a single, self contained binary for easy deployment
- Sources in literate format, using a Markdown variant

## Quick Start

For most systems (FreeBSD, NetBSD, macOS, Linux):

    make

You will need a standard C compiler and `make`. The `retro-ri`
binary requires (n)curses, but you can ignore any build/link
errors by doing:

    make -kis

If you are building an an older Linux system and are running
into problems, try using the `build.sh` script:

    sh build.sh

Or the alternate `Makefile.linux`:

    make -f Makefile.linux

## Executables

Binaries will be placed in the `bin` directory.

### retro

The primary executable is `retro`. This is used for running the
examples and the Atua (gopher) & Casket (http) servers that
power forthworks.com.

The `retro` executable embeds the image into the binary, making
it trivial to copy and deploy.

This interface layer also extends the language with many new
words and vocabularies, adds scripting, file i/o, gopher, and
floating point math support.

The `retro` executable can handle a variety of command line
arguments:

    retro

Starts the *listener*, a basic REPL for interactive use.

    retro -i

Starts the *listener*, a basic REPL for interactive use.

    retro -c

Starts the *listener*, a basic REPL for interactive use. With
`-c`, it runs in character breaking mode, where input is run
as soon as a space or enter is encountered. This requires RETRO
to have been built with TERMIOS support.

    retro filename

This will run the code in the specified file, then exit. This
is also used to run programs as shell-type scripts using a
header line like `#!/usr/bin/env retro`.

    retro -i -f filename

This will run the code in the specified file, then start the
listener.

    retro -c -f filename

This will run the code in the specified file, then start the
listener in character breaking mode.

    retro -h

Displays a summary of the command line arguments.

Source files for use with `retro` are written with code in
fenced blocks:

    commentary here
     
    ~~~
    code here
    ~~~

Anything outside the fenced blocks will be ignored.

### retro-repl

This is a basic `read-evaluate-print-loop` for interactive use.
It's intended for quick tests and as an easy starting point for
new interfaces.

The `retro-repl` interface looks for the RETRO image (normally
`ngaImage`) in the current working directory. If not found,
this will use an embedded copy.

### retro-ri

`retro-ri` is an interactive, curses based interface inspired
by the interface used around 2001 with RETRO4. It provides an
output area and then an input area at the bottom. Input is
processed as it is entered, and the top stack items appear to
the right of the input.

It's useful for playing around with new ideas and seeing how
the stack works.

## Commercial Versions

I provide proprietary versions of RETRO for iOS and macOS.
These use a custom, dual pane editor-based environment and
some platform specific words and extensions. They do use the
same image and virtual machine as `retro`, `retro-repl`, etc.

iOS:   https://itunes.apple.com/us/app/retro-forth-12/id1170943580?ls=1&mt=8

macOS: https://itunes.apple.com/us/app/retro-forth/id1317494014?ls=1&mt=12

The macOS application is also availble for free, though the
source is not published.

Proceeds from these are used to pay for various project related
expenses (servers, domains, hardware upgrades, and an
occasional cup of tea).

## Alternative Implementations

This source tree includes implementations in C#, JavaScript,
Pascal, and Python. These are not officially supported, but
are provided for your use if the C implementation will not
suffice.
