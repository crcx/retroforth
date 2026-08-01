# Examples

The `example/` directory is a collection of programs written in RETRO. They
are intended as inspiration and reference material: read, adapt, and learn
from them. They are not a test suite, nor are they necessarily tools meant to
be run unattended; some expect particular files, services, platforms, or
local configurations specific to the author.

The directory contains many more examples than the selection below, including
network clients and servers, editors, format converters, and experimental
utilities.

| Filename                     | Description                             |
| ---------------------------- | --------------------------------------- |
| 99-bottles.retro             | Display "99 bottles of beer" song       |
| adding-vectors.retro         | Add values in two arrays                |
| Atua-WWW.retro               | Atua HTTP server                        |
| Atua.retro                   | Atua Gopher server                      |
| autopsy.retro                | Debugging aids                          |
| cat.retro                    | Display a file to the output            |
| chess.retro                  | Simple ASCII-based chess game           |
| echo.retro                   | Echo command-line arguments             |
| gcd.retro                    | Greatest common denominator             |
| iterative-fibonacci.retro    | Fibonacci sequence, iterative           |
| least-common-multiple.retro  | Find least common multiple              |
| primes.retro                 | Prime sieve                             |
| recursive-factorial.retro    | Factorial of a number, recursive        |
| recursive-fibonacci.retro    | Fibonacci sequence, recursive           |
| roo.retro                    | Block editor using Tuporo backend       |
| 1D-Cellular-Automota.retro   | 1D cellular automata (Rosetta Code)     |
| is-palindrome.retro          | Is a string a palindrome?               |
| tuporo.retro                 | Block storage via Gopher protocol       |
| VT100.retro                  | Example VT100 namespace                 |
| sort-on-stack.retro          | Sort numbers on the stack               |
| accumulator.retro            | Sample use of `does`                    |
| parse-ups.retro              | Break a UPS tracking number apart       |
| rot13.retro                  | ROT13 "encryption"                      |
| hanoi.retro                  | Towers of Hanoi solver                  |
| string-to-number-with-base.retro | Numbers in miscellaneous bases      |
| dictionary-stats.retro       | Uses `d:for-each` to find information about word names |
| tokenize-string.retro        | Tokenize a string into a set            |
| unicode.retro                | Example showing Unicode (UTF-8)         |

## More examples by subject

### Language, data, and control flow

`ans-pick-roll.retro`, `assertions.retro`, `buffer.retro`, `bury.retro`,
`byte-addressing.retro`, `c-style-comments.retro`, `close-paren.retro`,
`compat.retro`, `defstruct.retro`, `dictionary-alias.retro`,
`dictionary-used-in.retro`, `display-names.retro`,
`display-word-location-information.retro`, `double.retro`, `enum.retro`,
`evaluate-string.retro`, `float-var.retro`, `floating-point-encoding.retro`,
`forth-style-comments.retro`, `hiding-words.retro`, `light-weight-flow-control.retro`,
`linked-list.retro`, `local-variables.retro`, `marker.retro`,
`namespaces.retro`, `naming-quotes.retro`, `numbers-without-prefix.retro`,
`numeric-ranges.retro`, `queue.retro`, `safety-net.retro`,
`sandboxed-dictionary.retro`, `save-and-restore-stack.retro`, `select.retro`,
`shared.retro`, `sort-on-stack.retro`, `string-to-number-with-base.retro`,
`top-of-address-stack.retro`, `trail.retro`, `unsigned.retro`,
`variables-and-formulas.retro`, and `vocabulary.retro` explore the language,
dictionary, stack, numeric, and data-structure facilities.

### Algorithms, puzzles, and visual programs

`1D-Cellular-Automota.retro`, `advent-of-code-2020-day-1.retro`,
`advent-of-code-2020-day-2.retro`, `advent-of-code-2020-day-3.retro`,
`advent-of-code-2020-day-4.retro`, `advent-of-code-2020-day-5.retro`,
`advent-of-code-2021-day-1.retro`,
`advent-of-code-2021-day-2.retro`, `chess.retro`, `conways-life.retro`,
`gcd.retro`, `hanoi.retro`, `is-pangram.retro`, `iterative-fibonacci.retro`,
`least-common-multiple.retro`, `magic-8th-ball.retro`, `mandelbrot.retro`,
`matrix.retro`, `palindromic-numbers.retro`, `primes.retro`, `quad.retro`,
`recursive-factorial.retro`, `recursive-fibonacci.retro`, `rng.retro`, and
`sea-level-rise.retro` provide compact problem-solving and display examples.

### Files, archives, text, and formats

`archive.retro`, `archive-info.retro`, `archive-extract.retro`, `cat.retro`,
`capture-output.retro`, `delete-file.retro`, `file.retro`, `json.retro`,
`markdown.retro`, `markdown-to-xhtml.retro`, `morse.retro`,
`pali-to-html.retro`, `parse-ups.retro`, `rot13.retro`, `strip-html.retro`,
`tokenize-string.retro`, `uudecode.retro`, `uuencode.retro`, and
`wordwrap.retro` demonstrate common input, output, encoding, and conversion
tasks.

### Unix, networking, and services

`7080.retro`, `alternate-listener.retro`, `Atua.retro`, `Atua-WWW.retro`,
`atua-gemini.retro`, `atua-gophermap.retro`, `Casket-HTTP.retro`,
`curl.retro`, `detect-devices.retro`, `gopher.retro`, `http-get.retro`,
`http-post.retro`, `irc-bot.retro`, `irc-logger.retro`, `mail.retro`,
`net-fetch.retro`, `paste.retro`, `paste-to-sprunge.retro`, `rfc865.retro`,
`rfc867.retro`, `shell.retro`, `socket-client.retro`, `socket-server.retro`,
`tuporo.retro`, `unix-does-user-exist.retro`, and `share.retro` show use of
Unix interfaces and network protocols.

### Editors, blocks, and RETRO tooling

`block-editor.retro`, `edit.retro`, `EDA.retro`, `ilo.retro`, `ilo-export.retro`,
`konilo-wiki.retro`, `reforth.retro`, `retro-edit.retro`,
`retro-embedimage.retro`, `retro-extend.retro`,
`retro-generate-image-js.retro`, `retro-locate.retro`, `retro-muri.retro`,
`retro-stats.retro`, `retro-tags.retro`, `retro-unu.retro`, `rilo-editor.retro`,
`roo.retro`, and `retro.retro` are larger applications or tools for working
with RETRO images, source, and block systems.

### Other applications and experiments

`amalgamate.retro`, `amalgamate-python.retro`, `calling-retro-from-c.c`,
`colored-dwords.retro`, `echo.retro`, `export-as-html.retro`,
`export-muri-as-html.retro`, `forget.retro`, `gott.retro`, `HTML.retro`,
`key-value-store.retro`, `minimize.retro`, `muri-with-hex.retro`,
`publish-examples.retro`, `retro.blocks.gz`, `toki-pona-translate.retro`,
`ulz.retro`, and `words-four-column.retro` cover
embedding, presentation, publication, and assorted experiments.

The `fsl/` directory contains a partial port of the Forth Scientific Library;
`iOS/` contains an iOS-specific Gopher client; and `sqlite3/` contains a
SQLite wrapper and example database program.
