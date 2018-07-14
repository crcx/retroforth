RETRO 12 - 2018.6 - WIP

Major Changes:

- renamed `s:with-format` to `s:format`
- renamed `puts` to `s:put`
- renamed `putn` to `n:put`
- renamed `putf` to `f:put`
- renamed `putc` to `c:put`
- renamed `getc` to `c:get`
- renamed `gets` to `s:get`
- renamed `words` to `d:words`
- ```` no longer used for code blocks (now reserved for 'tests'
  under rre)
- removed `tors`
- add `file:open<for-reading>` to RRE
- add `file:open<for-append>` to RRE
- add `file:open<for-writing>` to RRE


----------------------------------------------------------------

Bug fixes:

- use CELL instead of int in the VM
- glossary.forth now renders < > and & when serving the index as
  HTML

Build:

Core Language:

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

Interfaces:

- rre:

  - no longer use ```` for code blocks; use ~~~ instead
  - added `f:NAN`
  - added `f:INF`
  - added `f:-INF`
  - added `f:nan?`
  - added `f:inf?`
  - added `f:-inf?`
  - added `f:sqrt`
  - added "-t" command line argument to run tests
  - rewrote command line handler
  - added `file:open<for-reading>`
  - added `file:open<for-append>`
  - added `file:open<for-writing>`

- javascript/html

  - cleaned up user interface (closer to the iOS UI now)
  - simplified, cleaned up CSS a bit

Other:

Documentation:

Examples:

- fix numerous spelling errors (thanks hannah!)
- add example showing a means of addressing specific bytes
- add example showing assertions
- add example showing hooks/deferred words

Final Notes:
