# RETRO 12 - 2018.8

RETRO 12.2018.8 brings a significant number of changes to the
language. Of particular note is the renaming of I/O words to
group them into the standard namespaces. This will require a
modest amount of changes to existing sources.

## Bug fixes:

- use CELL instead of int in the VM
- glossary.forth now renders < > and & when serving the index as
  HTML

## Build

## Core Language

- renamed `s:with-format` to `s:format`
- renamed `puts` to `s:put`
- renamed `putn` to `n:put`
- renamed `putf` to `f:put`
- renamed `putc` to `c:put`
- renamed `getc` to `c:get`
- renamed `gets` to `s:get`
- renamed `words` to `d:words`
- add `d:words-with`
- add `set:make`
- add `{` and `}` to create sets
- removed `tors`

## Interfaces

- rre:

  - rewrote command line handler
  - added "-t" command line argument to run tests
  - no longer use ```` for code blocks; use ~~~ instead
  - use ```` for automated tests
  - added `f:NAN`
  - added `f:INF`
  - added `f:-INF`
  - added `f:nan?`
  - added `f:inf?`
  - added `f:-inf?`
  - added `f:sqrt`
  - added `f:round`
  - added `file:open<for-reading>`
  - added `file:open<for-append>`
  - added `file:open<for-writing>`

- javascript/html

  - cleaned up user interface (closer to the iOS UI now)
  - simplified, cleaned up CSS a bit

Other:

Documentation:

- Glossary expanded to cover the new words

Examples:

- fix numerous spelling errors (thanks hannah!)
- add example showing a means of addressing specific bytes
- add example showing assertions
- add example showing hooks/deferred words
- add Casket, an HTTP/1.1 server
- add example showing a means of encoding floating point
  values into integer cells

Final Notes:
