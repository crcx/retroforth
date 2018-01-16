RETRO 12 - 2018.1

Bug fixes:

- Fixed an issue with line endings in the Glossary's gopher server

Language Improvements:

- `s:with-format` now handles `\r`

Interfaces:

- rre

  - Added `f:ceiling`
  - Significant internal refactoring

    - Extended instruction set handling
    - Command line argument handling
    - Individual features can now be disabled easily

- repl

  - Build script no longer links this against libm
  - added varient for x86 DOS systems

- pascal/listener is a Free Pascal implementation by Rob Judd
- ri is a new (n)curses based interface

Other:

Documentation:

- Added significant comments to repl
- Added significant comments to rre
- Began adding examples to the Glossary
- Update code fence style

Examples:

Final Notes:
