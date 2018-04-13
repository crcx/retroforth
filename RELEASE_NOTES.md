RETRO 12 - 2018.4

Bug fixes:

Language Improvements:

- rx kernel: faster and smaller `s:eq?`
- stdlib: smaller implementations of `c:whitespace?` and `c:vowel?`

Interfaces:

- rre

  - add NOP skipping to Nga instruction handler
  - add `unix:io:putn`
  - add `unix:io:puts`
  - fixed handling of backspace
  - removed: listen-cbreak
  - now uses termios instead of calling out to stty for
    character breaking mode
  - now exits on stack overflow, stack underflow, memory write out of bounds

- repl

  - now embeds image (used if no ngaImage in the cwd)
  - add NOP skipping to Nga instruction handler

- native

  - Implementations of repl w/o requiring libc

    - FreeBSD / x86
    - FreeBSD / x86-64
    - Linux / x86
    - Standalone / x86

- ri

  - now uses same embedded image as repl
  - add NOP skipping to Nga instruction handler

- To-be-named

  - HTML + JavaScript implementation using iOS/macOS style editor

Other:

- fixed bugs in example/NumbersWithoutPrefix.forth (thanks to kiyoshi)
- source now builds using ancient (2.92) gcc
- back to using `make`

Documentation:

- fix sorting of Glossary by setting LC_ALL=c
- added words from RETRO/iOS
- fix descriptions for `until`, `while`

Examples:

- new debugger (examples/Autopsy.forth)
- added defstruct.forth

Final Notes:
