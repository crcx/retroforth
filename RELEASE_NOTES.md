# RETRO 12 - 2018.12

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

## Build

- add `Makefile.linux` to help with building on some Linux systems
- recreate `bin` if it's not present

## Core Language

- add `c:to-number`
- minor optimizations throughout
- `s:format` now treats `\0` as ASCII NUL
- added `prefix:|` for use with compiler macros
- inline some stack shufflers for better performance and code density
- add `set:counted-results`
- deprecate `set:from-results`

## Interfaces

### retro (formerly rre)

- moved Windows implementation to `interfaces/windows`
- now starts the listener when run w/o any arguments
- added `-s` command line parameter to silence input echo & ok prompt
- merged in the FloatingPointEncoding words (`u:` namespace)
- added `f:store`, `f:fetch`

## Tools

- added commentary to Unu
- Retro implementation of Unu
- Retro implementation of Muri

## Other

## Documentation

- Glossary expanded to cover the new words

## Examples

## Final Notes
