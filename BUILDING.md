# Building RetroForth

This is a quick overview of how to build Retro on a BSD, Linux,
or macOS system. It assumes you already have the requirements
(c compiler, make) setup in your command line environment.

## Standard Builds

A standard build should just require running `make`:

    make

This will build the toolchain and a binary runtime for retro.
These will be placed in the `bin` directory.

## Customized Builds

The basic system provides most of the functionality, but you
can enable or disable specific elements by editing either
the Makefile (on BSD) or GNUmakefile (or Linux or macOS).

Find the fourth section in the Makefile and either uncomment
or comment the ENABLED and DEVICES lines for the optional parts
you want to include in your build.

Most functionality is enabled by default. Optional things you
may wish to enable include sockets, ffi, and multicore.

## More Complex Customizations

There are additional things you can update. Take a look in the
manual for more details on this.
