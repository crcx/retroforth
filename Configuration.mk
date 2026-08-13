# -------------------------------------------------------------

# These are used when building and signing a release.

VERSION ?= 2024.10
KEYPAIR ?= 2024-10

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

# If using the Python toolchain, remove the -DBIT64 from the
# OPTIONS

OPTIONS ?=
#OPTIONS += -DBIT64

# This helps improve performance on some systems.

# OPTIONS += -DBRANCH_PREDICTION

# This may help improve performance, but not all compilers
# support it. (E.g., clang-1200 on some macOS systems does
# not support it).

# OPTIONS += -march=native

# Select a feature profile with `make PROFILE=...`. `full` preserves
# the historical default; callers may still append feature flags through
# ENABLED or replace the embedded interfaces through DEVICES.

OPTIONS += -DMAKEFILE_CONFIG

PROFILE ?= full
ENABLED ?=

PROFILE_FULL = \
	-DENABLE_FLOATS \
	-DENABLE_FILES \
	-DENABLE_UNIX \
	-DENABLE_RNG \
	-DENABLE_CLOCK \
	-DENABLE_SCRIPTING \
	-DENABLE_SIGNALS \
	-DENABLE_MULTICORE \
	-DENABLE_ERROR \
	-DENABLE_UNSIGNED \
	-DENABLE_MALLOC \
	-DENABLE_BLOCKS \
	-DENABLE_IOCTL

PROFILE_PORTABLE = \
	-DENABLE_FLOATS \
	-DENABLE_FILES \
	-DENABLE_CLOCK \
	-DENABLE_SCRIPTING \
	-DENABLE_ERROR \
	-DENABLE_UNSIGNED \
	-DENABLE_BLOCKS

PROFILE_MINIMAL = \
	-DENABLE_SCRIPTING

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
DEVICES += interface/hashed-dictionary.retro
DEVICES += interface/final.retro
DEVICES += interface/library.retro
DEVICES += interface/descriptions.retro

# -------------------------------------------------------------

GLOSSARY ?= ./bin/retro tools/glossary.retro
ASSEMBLE ?= ./bin/retro-muri
EXTEND   ?= ./bin/retro-extend
EXPORT   ?= ./bin/retro-embedimage
RETRO    ?= ./bin/retro

# -------------------------------------------------------------
