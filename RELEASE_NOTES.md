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

----------------------------------------------------------------

Bug fixes:

- use CELL instead of int in the VM

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

- javascript/html

  - cleaned up user interface (closer to the iOS UI now)
  - simplified, cleaned up CSS a bit

Other:

Documentation:

Examples:

- fix numerous spelling errors (thanks hannah!)
- add example showing a means of addressing specific bytes

Final Notes:
