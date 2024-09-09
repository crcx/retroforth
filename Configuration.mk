# -------------------------------------------------------------

# These are used when building and signing a release.

VERSION ?= 2024.9
KEYPAIR ?= 2024-09

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
OPTIONS += -DBIT64

# This helps improve performance on some systems.

# OPTIONS += -DBRANCH_PREDICTION

# This may help improve performance, but not all compilers
# support it. (E.g., clang-1200 on some macOS systems does
# not support it).

# OPTIONS += -march=native

# The I/O devices can be enabled or disabled. Comment or
# uncomment the corresponding ENABLED and DEVICES lines and
# then run `make`. Of particular note here, sockets support
# is disabled by default and you may wish to enable it.

# The I/O devices can be enabled or disabled. Comment or
# uncomment the corresponding ENABLED and DEVICES lines and
# then run `make`. Of particular note here, sockets support
# is disabled by default and you may wish to enable it.

OPTIONS += -DMAKEFILE_CONFIG

ENABLED ?=
ENABLED += -DENABLE_FLOATS
ENABLED += -DENABLE_FILES
ENABLED += -DENABLE_UNIX
ENABLED += -DENABLE_RNG
ENABLED += -DENABLE_CLOCK
ENABLED += -DENABLE_SCRIPTING
# ENABLED += -DENABLE_SOCKETS
ENABLED += -DENABLE_SIGNALS
ENABLED += -DENABLE_MULTICORE
# ENABLED += -DENABLE_FFI
ENABLED += -DENABLE_ERROR
ENABLED += -DENABLE_UNSIGNED
ENABLED += -DENABLE_MALLOC
ENABLED += -DENABLE_BLOCKS
ENABLED += -DENABLE_IOCTL

DEVICES ?=
DEVICES += interface/ll.retro
DEVICES += interface/dedup.retro
DEVICES += interface/stack-comments.retro
DEVICES += interface/sources.retro
DEVICES += interface/devices.retro
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
DEVICES += interface/error.retro
DEVICES += interface/ioctl.retro
DEVICES += interface/final.retro
DEVICES += interface/library.retro

# -------------------------------------------------------------

GLOSSARY ?= ./bin/retro tools/glossary.retro
ASSEMBLE ?= ./bin/retro-muri
EXTEND   ?= ./bin/retro-extend
EXPORT   ?= ./bin/retro-embedimage
RETRO    ?= ./bin/retro

# -------------------------------------------------------------
