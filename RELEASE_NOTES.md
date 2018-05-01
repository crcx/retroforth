RETRO 12 - 2018.6 - WIP

Major Changes:

- renamed `s:with-format` to `s:format`
- ```` no longer used for code blocks (now reserved for 'tests'
  under rre)

----------------------------------------------------------------

Bug fixes:

Build:

Core Language:

- renamed `s:with-format` to `s:format`

Interfaces:

- no longer use ```` for code blocks; use ~~~ instead
- rre:

  - added `f:NAN`
  - added `f:INF`
  - added `f:-INF`
  - added `f:nan?`
  - added `f:inf?`
  - added `f:-inf?`
  - added "-t" command line argument to run tests
  - rewrote command line handler

Other:

Documentation:

Examples:

- fix numerous spelling errors (thanks hannah!)

Final Notes:
