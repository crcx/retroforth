# RETRO 12.2019.6

## Bug Fixes

## Build

- parallel builds now work
- refactor Makefile

## Core Language

- remove `set:from-results`
- add `if;`
- add `-if;`
- add `hook`
- add `set-hook`
- add `unhook`
- `c:put` primitive is a hookable word

## Nga

## Interfaces

- use strlcpy, strlcat instead of strcpy and strcat
- include copies of the above for users of glibc
- add retro-compiler

### retro

### retro-compiler

- new interface for creating turnkey executables on bsd, linux

### retro-ri

- support switching between multiple copies of the image

## Tools

- use strlcpy, strlcat instead of strcpy and strcat
- retro-extend: cleaner output
- retro-extend: remove unused code
- retro-extend: allow multiple files
- retro-extend: reduce memory usage
- retro-extend: fix potential buffer overrun

## Other

## Documentation

## Examples

- add DisplayNames.forth
- add KeyValueStore.forth
- add net_fetch.forth
- add Save_and_Restore_Stack.forth
- add share.forth and shared.forth
- switch to dvorak key bindings in Roo.forth
- remove Hooks.forth (now in core language)

## Final Notes
