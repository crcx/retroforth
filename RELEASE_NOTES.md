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

## Bug fixes:

## Build

- add `Makefile.linux` to help with building on some Linux systems

## Core Language

- add `c:to-number`
- minor optimizations throughout
- `s:format` now treats `\0` as ASCII NUL

## Interfaces

### retro (formerly rre)

- moved Windows implementation to `interfaces/windows`
- now starts the listener when run w/o any arguments

Other:

Documentation:

- Glossary expanded to cover the new words

Examples:

Final Notes:
