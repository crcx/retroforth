# -------------------------------------------------------------

# These are used when building and signing a release.

VERSION ?= 2024.4
KEYPAIR ?= 2024-04

# -------------------------------------------------------------

# Installation targets

PREFIX ?= /usr/local
DATADIR ?= $(PREFIX)/share/RETRO12
DOCSDIR ?= $(PREFIX)/share/doc/RETRO12
EXAMPLESDIR ?= $(PREFIX)/share/examples/RETRO12
MANDIR ?= $(PREFIX)/man/man1

# -------------------------------------------------------------

# Flags for adding in libraries we need to link to.

# If not using floating point, you can remove the `-lm` from
# LIBM.

LIBM ?= -lm
LIBDL ?=

# -------------------------------------------------------------

OPTIONS ?=
OPTIONS += -DUSE_UTF32
#OPTIONS += -DBIT64

# The I/O devices can be enabled or disabled. Comment or
# uncomment the corresponding ENABLED and DEVICES lines and
# then run `make`. Of particular note here, sockets support
# is disabled by default and you may wish to enable it.

DEVICES ?=
DEVICES += interface/ll.retro
DEVICES += interface/dedup.retro
DEVICES += interface/sources.retro
DEVICES += interface/konilo.retro
DEVICES += interface/floatingpoint.retro
DEVICES += interface/filesystem.retro
DEVICES += interface/unix.retro
DEVICES += interface/rng.retro
DEVICES += interface/clock.retro
DEVICES += interface/scripting.retro
DEVICES += interface/sockets.retro
DEVICES += interface/multicore.retro
DEVICES += interface/ffi.retro
DEVICES += interface/unsigned.retro
DEVICES += interface/future.retro
DEVICES += interface/block.retro
DEVICES += interface/deprecated.retro
DEVICES += interface/double.retro
DEVICES += interface/malloc.retro
DEVICES += interface/error.retro
DEVICES += interface/final.retro

# -------------------------------------------------------------

GLOSSARY ?= ./bin/retro tools/glossary.retro
ASSEMBLE ?= ./bin/retro-muri
EXTEND   ?= ./bin/retro-extend
EXPORT   ?= ./bin/retro-embedimage
RETRO    ?= ./bin/retro

# -------------------------------------------------------------
