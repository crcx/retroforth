# RETRO 2019.6

This is the changelog for the development builds of Retro.
It is expected to be released as the stable system in early
June.

## Bug Fixes

## Build

- parallel builds now work
- refactor Makefile
- support for adding custom code to an image at build
  time (see package/list)
- only build the toolchain and `retro` binary by default
- fix an issue with rebuilding of image

## Core Language

- remove `set:from-results`
- add `if;`
- add `-if;`
- add `hook`
- add `set-hook`
- add `unhook`
- add `(`
- add `)`
- `c:put` primitive is a hookable word
- `set:` is now `a:`
- add `s:begins-with?`
- add `s:ends-with?`
- remove `d:words`
- remove `d:words-with`
- add `a:eq?`
- add `a:-eq?`
- add `a:append`
- add `a:prepend`
- add `a:left`
- add `a:right`
- add `a:middle`
- add `a:chop`
- add `a:begins-with?`
- add `a:ends-with?`

## I/O and Extensions

### Floating Point

- add f:adepth
- add f:between?
- add f:case
- add f:dec
- add f:drop-pair
- add f:dup-pair
- add f:dump-astack
- add f:dump-stack
- add f:inc
- add f:limit
- add f:max
- add f:min
- add f:nip
- add f:pop
- add f:push
- add f:rot
- add f:sign
- add f:E1
- add f:-shift
- add f:+shift
- add f:signed-sqrt
- add f:+encode
- add f:-encode
- add f:signed-square
- improved f:tuck
- improved f:over
- u: is now e:

  - f:to-u     ->  f:to-e
  - u:-INF     ->  e:-INF
  - u:-inf?    ->  e:-inf?
  - u:INF      ->  e:INF
  - u:MAX      ->  e:MAX
  - u:MIN      ->  e:MIN
  - u:NAN      ->  e:NAN
  - u:clip     ->  e:clip
  - u:inf?     ->  e:inf?
  - u:max?     ->  e:max?
  - u:min?     ->  e:min?
  - u:n?       ->  e:n?
  - u:nan?     ->  e:nan?
  - u:to-f     ->  e:to-f
  - u:zero?    ->  e:zero?

- add e:put
 
### Unix

- add unix:count-files-in-cwd
- add unix:for-each-file
- add unix:get-cwd
- add unix:time

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
- add `sys:name`
- will now display an error and exit on stack over/underflow
- `-c` is now `-i,c`
- added `-i,fs` (interface formerly existing as `retro-ri`)
- add `d:words`
- add `d:words-with`
- add `d:words-beginning-with`
- add `clock:day`
- add `clock:month`
- add `clock:year`
- add `clock:hour`
- add `clock:minute`
- add `clock:second`

### retro-compiler

- new interface for creating turnkey executables on bsd, linux

### retro-ri

- this interface has been rolled into the main `retro` binary

### native

- add port i/o words
- add cmos rtc driver
- add serial port driver
- now builds properly on openbsd
- add ata driver
- add block editor
- rewrite vga text driver in retro

## Tools

- use strlcpy, strlcat instead of strcpy and strcat
- retro-extend: cleaner output
- retro-extend: remove unused code
- retro-extend: allow multiple files
- retro-extend: reduce memory usage
- retro-extend: fix potential buffer overrun
- muri: remove the unused `c` directive
- add retro-describe

## Other

## Documentation

- add a collection of papers
- refactor glossary tool
- add HTML version of the Glossary
- add more concise text copies of the Glossary
- add man pages
- add BUILDING.md
- add missing descriptions
- add RETRO-Book.md

## Examples

- add Abort.forth
- add ANS-PICK-ROLL.forth
- add atua-gophermap.forth
- add Block-Editor.forth
- add Buffer.forth
- add CaptureOutput.forth
- add CloseParen.forth
- add DisplayNames.forth
- add DictionaryAlias.forth
- add DictionaryUsedIn.forth
- add EDA.forth
- add EvaluateString.forth
- add export-as-html.forth
- add FloatVar.forth
- add Forget.forth
- add HTML.forth
- add KeyValueStore.forth
- add Marker.forth
- add mail.forth
- add NamingQuotes.forth
- add NetFetch.forth
- add paste.forth
- add PasteToSprunge.forth
- add retro-extend.forth
- add retro-embedimage.forth
- add Sandboxed-Dictionary.forth
- add SaveAndRestoreStack.forth
- add share.forth and shared.forth
- add strip-html.forth
- add TokiPona-Translate.forth
- add uuencode.forth
- add uudecode.forth
- fix test block in defstruct.forth
- switch to dvorak key bindings in Roo.forth
- remove Hooks.forth (now in core language)
- improved edit.forth from Kiyoshi
- Casket-HTTP now supports server side scripting
- Fix a bug in Autopsy.forth causing dissassembly to
  list addresses off by one

## Final Notes
