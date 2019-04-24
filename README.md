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

For most systems (FreeBSD, NetBSD, OpenBSD, macOS, Linux):

    make

You will need a standard C compiler and `make`. The `retro-ri`
binary requires (n)curses, but you can ignore any build/link
errors by doing:

    make -kis

If you are building on an older Linux system and are running
into problems, try using the alternative Makefile:

    make -f Makefile.linux

## Executables

Binaries will be placed in the `bin` directory.

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

    retro -i

Starts the *listener*, a basic REPL for interactive use.

    retro filename

This will run the code in the specified file, then exit. This
is also used to run programs as shell-type scripts using a
header line like `#!/usr/bin/env retro`.

    retro -i -f filename

This will run the code in the specified file, then start the
listener.

    retro -h

Displays a summary of the command line arguments.

Source files for use with `retro` are written with code in
fenced blocks:

    commentary here
     
    ~~~
    code here
    ~~~

Anything outside the fenced blocks will be ignored.

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

This source tree includes additional implementations in C#,
JavaScript, Pascal, and Python. These are not officially
supported, but are provided for your use if the C
implementations will not suffice.
