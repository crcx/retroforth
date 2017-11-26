# Building RETRO

## Unix (BSD, Linux, macOS, Cygwin)

Requirements:

- a generic shell in `/bin/sh`
- standard unix tools (cp, rm, mv, cd)
- a c compiler (as `cc`)
- a standard libc and libm
- a unix-style host

You can either use `make` or just run `./build.sh` directly.

The resulting files will be in the `bin` directory: `rre` and
`repl`.

RETRO is known to build out of the box on FreeBSD, macOS, and
Linux. It'll also build fine under Cygwin if you are using a
Windows system.

## Windows

Requirements:

- .NET Framework

Process:

    %windir%\Microsoft.NET\Framework\v2.0.50727\csc.exe implementations\repl.cs

Copy the resulting `repl.exe` and the `ngaImage` to the same directory
and run the `repl.exe`
