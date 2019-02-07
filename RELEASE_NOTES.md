# RETRO 12.2019.6

This is the changelog for the development builds of Retro.
I am currently planning to have the next release occur in
June 2019.

## Bug Fixes

## Build

- parallel builds now work
- refactor Makefile
- support for adding custom code to an image at build
  time (see packages/list)
- only build the toolchain and `retro` binary by default

## Core Language

- remove `set:from-results`
- add `if;`
- add `-if;`
- add `hook`
- add `set-hook`
- add `unhook`
- `c:put` primitive is a hookable word

## I/O and Extensions

### Floating Point

- add f:push
- add f:pop
- add f:dup-pair
- add f:drop-pair
- add f:rot
- add f:nip
- add f:min
- add f:max
- add f:limit
- add f:between?
- add f:inc
- add f:dec
- add f:case
- add f:dump-stack
- improved f:tuck
- improved f:over

### Unix

- add unix:get-cwd
- add unix:count-files-in-cwd
- add unix:for-each-file

## Nga

## Interfaces

- use strlcpy, strlcat instead of strcpy and strcat
- include copies of the above for users of glibc
- add retro-compiler
- add random number generator

### retro

- add random number generator
- add `-u filename` startup parameter to allow selection of
  image file
- add `image:save`

### retro-compiler

- new interface for creating turnkey executables on bsd, linux

### retro-ri

- support switching between multiple copies of the image
- add random number generator
- add image:save

## Tools

- use strlcpy, strlcat instead of strcpy and strcat
- retro-extend: cleaner output
- retro-extend: remove unused code
- retro-extend: allow multiple files
- retro-extend: reduce memory usage
- retro-extend: fix potential buffer overrun
- muri: remove the unused `c` directive

## Other

## Documentation

- add a collection of papers
- refactor glossary tool
- add HTML version of the Glossary
- add more concise text copies of the Glossary
- add man pages
- add BUILDING.md

## Examples

- add Abort.forth
- add atua-gophermap.forth
- add Buffer.forth
- add CaptureOutput.forth
- add DisplayNames.forth
- add EvaluateString.forth
- add Forget.forth
- add HTML.forth
- add KeyValueStore.forth
- add Marker.forth
- add Naming_Quotes.forth
- add net_fetch.forth
- add paste.forth
- add Paste_to_Sprunge.forth
- add retro-extend.forth
- add retro-embedimage.forth
- add Save_and_Restore_Stack.forth
- add share.forth and shared.forth
- add uuencode.forth
- add uudecode.forth
- switch to dvorak key bindings in Roo.forth
- remove Hooks.forth (now in core language)
- improved edit.forth from Kiyoshi

## Final Notes
