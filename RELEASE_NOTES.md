RETRO 12 - 2017.12 (WIP)

Bug fixes:

- Now builds using GCC (thanks to wuehlmaus in the IRC channel for discovering a bug related to the order of linker arguments when dealing with GCC)
- Now builds on older Ubuntu systems with broken GNU make.

Language Improvements:

- ASCII constants are now using `class:data` (reducing image size by nearly 500 cells and removing call/return overhead when used)
- `s:with-format` now supports \_
- Added `s:split-on-string`
- Added `s:index-of-string`
- Added `s:replace`
- Added `s:tokenize`
- Added `s:tokenize-on-string`

Interfaces:

- *RRE* has significant improvements.

	- a `unix:` namespace with access to system-specific functionality:

		- `unix:system`
		- `unix:popen`
		- `unix:pclose`
		- `unix:fork`
		- `unix:wait`
		- `unix:exec0`
		- `unix:exec1`
		- `unix:exec2`
		- `unix:exec3`
		- `unix:getpid`
		- `unix:kill`
		- `unix:getenv`
		- `unix:putenv`
		- `unix:chdir`
		- `unix:write`
		- `unix:sleep`

	- Additions to `f`:

		- `f:sin`
		- `f:cos`
		- `f:tan`

	- Addtions to `file`:

		- `file:spew`

	- Interactive mode when launched with `-i`:

    rre -i

	- `bin/listener` now uses `rre` directly and no longer has to copy the RETRO code into a temporary file.

Documentation:

- Added a `Building.md` with some notes on the build process.
- Fixed formatting issues in the `LICENSE.md` (thanks to Mateusz Piotrowski)
- Build now uses a shell script instead of multiple Makefiles
- Added a database (tab separated data) of the words
- Added a `glossary` tool to search, edit, and extract data from the words database

Examples:

- Added `edit.forth`, a functional text/code editor

