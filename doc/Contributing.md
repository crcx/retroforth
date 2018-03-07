# Contributing

RETRO is an open-source project, but I am opinionated, and as I am
developing this primarily for my use, I may be picky when it comes
to adding changes to the core language.

I use Fossil to manage the RETRO source, but I do not accept pushes
to this directly. Patches or updated files can be emailed to
crc@forthworks.com. I will review and merge them manually.

## Interfaces

When making changes, I'm most open to extensions as part of the RRE
interface. (interfaces/rre.c, interfaces/rre.forth).

New interfaces are welcome. If they require more than 1-2 files, or
specific build systems (apart from BSD compatible `make` or `bin/sh`
scripts), please keep them in a separate subdirectory.

I'm not planning to accept new I/O or extensions to the basic REPL
(interfaces/repl.c) as this serves as a template for starting new
interfaces or implementations.

Changes or additions to the provided words must be documented in the
glossary (words.tsv).

## Kernel

The kernel (literate/Rx.md) is basically done. I'm still making small
changes in terms of tuning size/performance, but will not merge any
alterations to this apart from bug fixes currently.

## Standard Library

I may accept additions to the standard library. Words requiring any
I/O other than `putc` will not be accepted in the standard library.

Changes to the words must be documented in the glossary (words.tsv).

## Examples

I'm quite willing to add new examples or changes / improvements to
existing ones.

## Legal

All contributions to the main sources (existing interfaces, kernel,
standard library) must be public domain or under the ISC license
(see LICENSE.md). Please add the appropriate copyright notices to
the LICENSE and/or source files.

New examples or tools can be under similar licenses. Include the
copyright and license text in the sources.

I will not accept contributions under the GPL, LGPL, AGPL, or other
licenses that are too long to reasonably embed in a source file.

## Building

I do builds under GCC and CLANG on FreeBSD, NetBSD, Linux, macOS,
and Haiku. I build RRE for Windows using TCC on a Windows box.
Changes to the source must build cleanly on all test systems.

Note that Haiku uses GCC 2.95.3 for backwards compatibility with
BeOS R5 so sources must compile on this as well as modern compilers.
