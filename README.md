## RETRO

RETRO is a modern, pragmatic Forth drawing influences from many sources.

Features:

- Open Source (ISC License)
- Portable (runs on a MISC-style virtual machine)
- Small source & binaries
- Builds into a single, self contained binary for easy deployment
- Sources in literate format, using a Markdown variant

The language is a Forth dialect drawing influence from some of Chuck
Moore's newer ideas. It's clean, elegant, tiny, and easy to grasp and
adapt to various uses.

## Quick Start

For FreeBSD, macOS, Linux:

    make

Binaries will be found in the `bin` directory. The ones of interest
are:

    rre

Short for `run retro and exit`, this is the primary interface for RETRO.
The `rre` interface is used to run the examples and Atua Gopher and HTTP
servers that power forthworks.com.

`rre` embeds the RETRO image into the binary, making it trivial to copy
and deploy.

    repl

This is a basic `read-evaluate-print-loop` for interactive use. It's
intended for quick tests and as an easy starting point for new interfaces.
The `repl` interface requires the RETRO image (`ngaImage`) in the current
working directory to operate.

    listener

Wrapping a shell script, some RETRO code, and using the `rre` interface,
this is a slightly nicer alternative to the basic `repl` for interactive
use.

