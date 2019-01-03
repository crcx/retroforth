# RETRO 12.2019.1

In this release, the executables have been renamed to avoid
naming conflicts with other applications and packages.

    old name       new name
    ==========     ================
    rre            retro
    embedimage     retro-embedimage
    extend         retro-extend
    muri           retro-muri
    repl           retro-repl
    ri             retro-ri
    unu            retro-unu

The addition of the new `|` prefix makes compiler macros
easier to write. Contrast:

    :IF   &[ class:macro ; immediate
    :THEN &] class:macro &choose class:word ; immediate
    
    :IF   |[ ; immediate
    :THEN |] |choose ; immediate

## Bug Fixes

- `s:empty` now returns a terminated string

## Build

- add `Makefile.linux` to help with building on some Linux systems
- recreate `bin` if it's not present

## Core Language

- add `c:to-number`
- minor optimizations and cleanups throughout
- `s:format` now treats `\0` as ASCII NUL
- added `prefix:|` for use with compiler macros
- inline select words for better performance and code density
- add `set:counted-results`
- deprecate `set:from-results`

## Nga

- new instructions

  - add `ie` (i/o enumerate)
  - add `iq` (i/o query)
  - add `ii` (i/o interact)

## Interfaces

### retro (formerly rre)

- moved Windows implementation to `interfaces/windows`
- now starts the listener when run w/o any arguments
- added `-s` command line parameter to silence input echo & ok prompt
- merged in the FloatingPointEncoding words (`u:` namespace)
- added `f:store`, `f:fetch`

### retro.py

- fixed the bug causing it to run at a glacial pace

### barebones

- a small repl minimizing the C portion of the code

### listener

- removed. use `retro` or `retro-repl` instead

## Tools

- added commentary to Unu
- Retro implementation of Unu
- Retro implementation of Muri
- added Muri support for I/O instructions

## Other

- builds no longer fail if `bin/` does not exist

## Documentation

- Glossary expanded to cover the new words

## Examples

- atua now caps selector length at 255 characters (as per spec)
- atua no longer needs to read in entire file before sending
- prime sieve now does primes <= 3000 (no longer crashing when
  using default memory size)
- added matrix example
- added lightweight flow control words
- gopher client now works on the standard `retro` interface

## Final Notes

While this release has taken longer than expected to finish, I
am very pleased with it. The better naming of binaries, new I/O
model, and numerous small optimizations in the core language
make it better than ever.
