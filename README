          d8888b. d88888b d888888b d8888b.  .d88b.
          88  `8D 88'     `~~88~~' 88  `8D .8P  Y8.
          88oobY' 88ooooo    88    88oobY' 88    88
          88`8b   88~~~~~    88    88`8b   88    88
          88 `88. 88.        88    88 `88. `8b  d8'
          88   YD Y88888P    YP    88   YD  `Y88P'

--------------------------------------------------------------

Retro is a modern, pragmatic Forth drawing influences from
many sources. It's clean, elegant, tiny, easy to grasp, and
adaptable to many tasks.

--------------------------------------------------------------

## Features

- Open Source (ISC License)
- Portable (runs on a MISC-style virtual machine)
- Small source & binaries
- Builds into a single, self contained binary
- Sources in literate format, using a Markdown variant

--------------------------------------------------------------

## Quick Start

For most systems (FreeBSD, NetBSD, OpenBSD, macOS, Linux):

    make

You will need a standard C compiler and `make`.

--------------------------------------------------------------

## Executables

Binaries will be placed in the `bin` directory.

The primary executable is `retro`. This is used for running the
examples and the Atua (gopher) & Casket (http) servers that
power retroforth.org.

The `retro` executable embeds the image into the binary, making
it trivial to copy and deploy.

This interface layer also extends the language with many new
words and vocabularies, adds scripting, files, floating point,
and more.

The `retro` executable can handle a variety of command line
arguments which are covered in the manpage, but the most common
ones are:

    retro

Starts the *listener*, a basic REPL for interactive use.

    retro filename

This will run the code in the specified file, then exit. This
is also used to run programs as shell-type scripts using a
header line like `#!/usr/bin/env retro`.

    retro -i -f filename

This will run the code in the specified file, then start the
listener.

--------------------------------------------------------------

## Literate Sources

Source files for use with `retro` are written with code in
fenced blocks:

    commentary here

    ~~~
    code here
    ~~~

Anything outside the fenced blocks will be ignored.

--------------------------------------------------------------

## Documentation

The primary documentation is in RETRO-Book.md (and the formatted
epub.) Additional notes can be found in the `doc` directory.

--------------------------------------------------------------

## Alternative Implementations

In addition to the C and Python implementations, this source
tree includes additional implementations in C#, Nim, JavaScript,
and Pascal. These are not as well tested or as feature complete
as the main implementations, but are provided for your use if
the standard implementations will not suffice.

--------------------------------------------------------------

## Learn More

Visit http(s)://retroforth.org to learn more and follow along
with development.

--------------------------------------------------------------

## Patreon

I have a Patreon at https://www.patreon.com/_crc for those that
want to financially support development. All funds raised via
Pateron are put into development related expenses (server
expenses, app store fees, hardware).

Thanks go out to my current and past patrons:

- Kiyoshi YONEDA
- Krinkleneck
- Rick Carlino
- Scott McCallum
- Bartels Jon Randy
- Nuno
- Eli
- Brad S
