# Contributing

RETRO is an open-source project, but I am opinionated and may be
picky when accepting changes to the core language.

## Repositories

### Fossil

I use fossil to manage the RETRO source.

URL: http://forthworks.com:8000

### Git

I also provide a git mirror of the fossil repository.

* Sourcehut: https://git.sr.ht/~crc_/retroforth
* Github: https://github.com/crcx/retroforth.git

## Mailing Lists

There are three mailing lists.

- https://lists.sr.ht/~crc_/retroforth-announce
- https://lists.sr.ht/~crc_/retroforth-discuss
- https://lists.sr.ht/~crc_/retroforth-devel

## Bug Reports

You can email bug reports directly to me, report them in the #retro
IRC channel, or submit them to the online bug tracker at
https://sr.ht/~crc_/retroforth/trackers

## General Structure

There is a VM (implementations are in `vm`) named Nga. This implements
a MISC style processor and system specific I/O extensions.

The VM runs an image (implemented primarily in `image`, from
`retro.muri` and `retro.forth`) that provides the actual language
and a standard library of words.

System specific I/O extensions are provided and can be found in the
`interface` directory. These include things related to the actual
user interfaces, as well as wrappers to the I/O devices implemented
in the VM.

Examples can be found in the `example` directory. These are converted
to HTML with syntax highlighting and are published at http://forth.works

Documentation can be found in the `doc` directory. The Glossary is
managed via a CSV formatted database (`doc/words.csv`) which is edited
by `tools/glossary.retro`.

## Legal

All contributions to the main sources (existing interfaces, kernel,
standard library) must be public domain or under the ISC license
(see LICENSE.md). Please add the appropriate copyright notices to
the LICENSE and/or source files.

New examples or tools can be under similar liberal licenses. Include
the copyright and license text in the sources.

I will not accept contributions under the GPL, LGPL, AGPL, or other
licenses that are too long to reasonably embed in the header of a
source file.

## Building

I do builds under GCC and CLANG on FreeBSD, NetBSD, Linux, macOS,
and Haiku. I build RRE for Windows using TCC on a Windows box.
Changes to the source must build cleanly on all test systems.

Note that Haiku uses GCC 2.95.3 for backwards compatibility with
BeOS R5 so sources must compile on this as well as modern compilers.
