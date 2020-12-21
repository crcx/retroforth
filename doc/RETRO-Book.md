# RETRO: a Modern, Pragmatic Forth

Welcome to RETRO, my personal take on the Forth language. This
is a modern system primarily targetting desktop, mobile, and
servers, though it can also be used on some larger (ARM, MIPS32)
embedded systems.

The language is Forth. It is untyped, uses a stack to pass data
between functions called words, and a dictionary which tracks
the word names and data structures.

But it's not a traditional Forth. RETRO draws influences from
many sources and takes a unique approach to the language.

RETRO has a large vocabulary of words. Keeping a copy of the
Glossary on hand is highly recommended as you learn to use RETRO.

This book will hopefully help you develop a better understanding
of RETRO and how it works.

# Obtaining RETRO

## Stable Releases

I periodically make stable releases. This will typically happen
quarterly.

- http://forthworks.com/retro
- http://forth.works

## Snapshots

A lot of development happens between releases. I make snapshots
of my working source tree nightly (and often more often).

The latest snapshot can be downloaded from the following stable
URLs:

* http://forthworks.com/retro/r/latest.tar.gz
* gopher://forthworks.com/9/retro/r/latest.tar.gz

## Fossil Repository

I use a Fossil repository to manage development. To obtain a
copy of the repository install Fossil and:

    fossil clone http://forthworks.com:8000 retro.fossil
    mkdir retro
    cd retro
    fossil open /path/to/retro.fossil

See the Fossil documentation for details on using Fossil to
keep your local copy of the repository current.

This will let you stay current with my latest changes faster
than the snapshots, but you may occasionally encounter bigger
problems as some commits may be in a partially broken state.

If you have problems, check the version of Fossil you are
using. I am currently using Fossil 2.10, you may experience
issues checking out or cloning if using older versions.

## git Repository

There is now a read-only mirror of the fossil repository
provided via git. This is hosted on sr.ht.

    git clone https://git.sr.ht/~crc_/retroforth

## Notes

I personally recommend using either a recent snapshot or a
build from one of the repositories. This will reflect the
latest system as I use it, and are normally reliable as I
run them daily for my production systems.

# Building on BSD, Linux, macOS, and outher Inix Targets

RETRO is well supported on BSD (tested on FreeBSD, NetBSD,
OpenBSD), Linux, and macOS systems. It should build on any
of these without issue.

## Requirements

- c compiler & linker
- standard headers
- make

## Process

For a standard 32-bit system:

Run `make`

For a 64-bit system:

Run `make OPTIONS=-DBIT64`

This will build the toolchain and then the main `retro`
executable.

## Executables

In the `bin/` directory:

    retro
    retro-unu
    retro-muri
    retro-extend
    retro-embedimage
    retro-describe

## Platform Specific Notes

In addition to the 64-bit build, it is possible to override the
image size, address stack depth, and data stack depth by defining
the appropriate elements.

E.g., for a 64-bit build with:

    4,000,000 cells of memory
    4,000     item limit on data stack
    500       item limit on address stack

Run `make OPTIONS="-DBIT64 -DIMAGE_SIZE=4000000 -DSTACK_DEPTH=4000 -DADDRESSES=500"`


### Haiku

To build on Haiku, you need to link with the *network* library.
E.g.:

    make LDFLAGS=-lnetwork

## Issues

If you run into any build issues, please send details to
crc@forth.works so I can work on addressing them as quickly
as possible.

# Building RETRO on Windows

It is possible to build RETRO on Windows, though a few of the
extensions are not supported:

- no `unix:` words
- no `gopher:` words

This is currently more difficult than on a Unix host. If you have
Windows 10 and WSL, it may be better to build under that (using
the Unix instructions).

## Setup Build Environment

RETRO on Windows is built with TCC.

Go to http://download.savannah.gnu.org/releases/tinycc/

Download the *winapi-full* and *tcc-xxxx-bin* packages for your
system. Decompress them, copy the headers from the winapi
package into the tcc directory.

## Prepare Source

Copy the `source/interfaces/retro-windows.c` and the
`source/interfaces/retro-windows.c` to the directory you setup
tcc into.

## Build

Building will require use of the command line. Assuming that
tcc.exe is in the current directory along with the RETRO sources:

    tcc retro-windows.c -o retro.exe

# Building Alternative Systems

In addition to the C implementation, there are a few other
interfaces that can be built.

## Requirements

- c compiler (tested: clang, tcc, gcc)
- make
- standard unix shell

## retro-repl

A basic interactive system can be built by using:

    make bin/retro-repl

This requires a copy of `ngaImage` to be in the current
directory.

## Barebones

This is a minimal version of the `retro-repl`. It keeps the C
portion as short as possible, making it a useful starting point
for new interfaces.

To build:

    make bin/retro-barebones

## retro-compiler

This is a turnkey compiler. It can compile a new executable
bundling a Retro VM and image.

Requirements:

- BSD or Linux
- objcopy in $PATH

To build:

    make bin/retro-compiler

Example use:

1. Given a source file like "Hello.forth":

    ~~~
    :hello 'hello_world! s:put nl ;
    ~~~

2. Use:

    ./bin/retro-compiler Hello.forth hello

The first argument is the source file, the second is the
word to run on startup.

3. Run the generated `a.out`

Limits:

This only supports the core words ('all' interface) and the
file i/o words. Support for other I/O extensions will be
added in the future.

## Pascal

There is a Pascal version of `retro-repl`.

Dependencies:

- freepascal

Building:

    cd vm/nga-pascal
    fpc listener.lpr

This will require a copy of the `ngaImage` in the
current directory.

## Python: retro.py

This is an implementation of `retro-repl` in Python. As
with `retro-repl` it requires the `ngaImage` in the current
directory when starting.

## C#: retro.cs

This is an implementation of `retro-repl` in C#. As with
`retro-repl` it requires the `ngaImage` in the current
directory when starting.

Building:

    cd vm\nga-csharp
    csc retro.cs

You'll need to make sure your path has the CSC.EXE in it,
or provide a full path to it. Something like this should
reveal the path to use:

    dir /s %WINDIR%\CSC.EXE

I've only tested building this using Microsoft's .NET tools.
It should also build and run under Mono.

# Advanced Builds

## Custom Image

For users of BSD, Linux, macOS, you can customize the image at
build time.

In the top level directory is a `package` directory containing
a file named `list.forth`. You can add files to compile into
your system by adding them to the `list.forth` and rebuilding.

Example:

If you have wanted to include the NumbersWithoutPrefixes.forth
example, add:

    ~~~
    'example/NumbersWithoutPrefixes.forth include
    ~~~

To the start of the `list.forth` file and then run `make` again.
The newly built `bin/retro` will now include your additions.

# Starting RETRO

RETRO can be run for scripting or interactive use.

## Interactive

To start it interactively, run: `retro` without any command line
arguments, or with `-i`, `-s`, or `-i,c`.

Starting the interactive system:

```
retro
```

Or:

```
retro -i
```

This should be sufficient for most uses.

Starting the interactive system (without displaying the
startup banner):

```
retro -s
```

## Using In a Pipe

If using a Unix shell and piping input between processes, you
will probably want to use `-s` to supress the startup messages
and `Ok` prompt that normally appear.

E.g.,

```
echo "'lol s:put nl" | retro -s
```

## Running A Program In A File

You can run code in a file very easily. This is simply:

```
retro filename
```

You can follow the filename with any arguments that it may need.
These will be accessible to the program via the `script:arguments`
and `script:get-argument` words.

Source files must be written in Unu format.

## Scripting

You can use RETRO to write scripts. Add a shebang:

```
#!/usr/bin/env retro
```

And make the file executable.

Source files must be written in Unu format.

## Command Line Arguments

For a summary of the full command line arguments available:

    Scripting Usage:

        retro filename [script arguments...]

    Interactive Usage:

        retro [-h] [-i] [-c] [-s] [-f filename] [-t]

      -h           Display this help text
      -i           Interactive mode (line buffered)
      -s           Suppress the startup text
      -f filename  Run the contents of the specified file
      -t           Run tests (in ``` blocks) in any loaded files

# Basic Interactions

Start RETRO in interactive mode:

```
retro -i
```

You should see something similar to this:

    RETRO 12 (2019.6)
    8388608 MAX, TIB @ 1025, Heap @ 9374

At this point you are at the *listener*, which reads and
processes your input. You are now set to begin exploring
RETRO.

To exit, run `bye`:

```
bye
```

# Unu: Simple, Literate Source Files

RETRO is written in a literate style. Most of the sources
are in a format called Unu. This allows easy mixing of
commentary and code blocks, making it simple to document
the code.

As an example,

    # Determine The Average Word Name Length

    To determine the average length of a word name two values
    are needed. First, the total length of all names in the
    Dictionary:

    ~~~
    #0 [ d:name s:length + ] d:for-each
    ~~~

    And then the number of words in the Dictionary:

    ~~~
    #0 [ drop n:inc ] d:for-each
    ~~~

    With these, a simple division is all that's left.

    ~~~
    /
    ~~~

    Finally, display the results:


    ~~~
    'Average_name_length:_%n\n s:format s:put
    ~~~

This illustrates the format. Only code in the fenced blocks
(between \~~~ pairs) get extracted and run.

(Note: this only applies to source files; fences are not used
when entering code interactively).

## On The Name

The name Unu comes from the Maori language, where it means:

    (verb) (-hia) pull out, withdraw, draw out, extract.
    Taken from https://maoridictionary.co.nz/

# RETRO's Markdown Syntax

I use a variation of Markdown for writing documentation and
when commenting code written in RETRO. The syntax is
described below.

## Basic Syntax

### Headings

Headings start with one or more number (`#`) signs. The
number of number signs should correspond to the heading
level.

    # Heading 1
    ## Heading 2
    ### Heading 3
    #### Heading 4

My Markdown does not support the alternate underline
format for headings.

### Paragraphs & Line Breaks

To create paragraphs, use a blank line to separate one or
more lines of text.

Do not add spaces or tabs at the start of a paragraph as
this may cause the Markdown tools to interpret the line
improperly.

Line breaks are entered at the end of each line.

### Emphasis

#### Bold

To make text bold, surround it with asterisks.

    The *bold* word.

#### Italic

To make text italic, surround it with front slashes.

    The /italic words/.

#### Underline

To underline text, surround it with underscores.

    Underline _some text_.

### Horizontal Rules

Horizontal rules can be inserted by starting a line with a
sequence of four or more dashes (`-`) or four or more alternating
dash and plus (`-+-+`) characters.

    ----

## Lists

Lists start with a `-` or `*`, followed by a space, then the item
text. Additionally, nested lists starting with two spaces before
the list marker can be used.

    - this is a list item
    - so is this

      - this will be indented
      - likewise

    - back to the standard level

## Code

### Code Blocks

Code blocks start and end with ~~~ on a line by themselves.

    Sum the values.

    ~~~
    { #10 #20 #13 #4 #22 } #0 [ + ] a:reduce
    ~~~

You can also denote code by starting the line with four spaces.

        This line will be treated as code.

### Test Blocks

Unit testing blocks start and end with ``` on a line by
themselves.

    ```
    { #10 #20 #13 #4 #22 } #0 [ + ] a:reduce
    ```

### Inline Code

To mark a sequence as inline code, surround it with backticks.

    For instance, look at the value in `Compiler` to see if
    the colon compiler is active.

## Escaping

You can preceed a character with a backslash (\\) to have it
not be processed as a Markdown element.

# A Quick Tutorial

Programming in RETRO is all about creating words to solve
the problem at hand. Words operate on data, which can be
kept in memory or on the stack.

Let's look at this by solving a small problem: writing a
word to determine if a string is a palindrome.

A palindrome is a phrase which reads the same backward
and forward.

We first need a string to look at. Starting with something
easy:

```
'anna
```

Looking in the Glossary, there is a `s:reverse` word for
reversing a string. We can find `dup` to copy a value, and
`s:eq?` to compare two strings. So testing:

```
'anna dup s:reverse s:eq?
```

This yields -1 (`TRUE`) as expected. So we can easily
name it:

```
:palindrome? dup s:reverse s:eq? ;
```

Naming uses the `:` prefix to add a new word to the dictionary.
The words that make up the definition are then placed, with a
final word (`;`) ending the definition. We can then use this:

```
'anna palindrome?
```

Once defined there is no difference between our new word and
any of the words already provided by the RETRO system.

# Syntax

RETRO has more syntax than a traditional Forth due to ideas
borrowed from ColorForth and some design decisions. This has
some useful traits, and helps to make the language more
consistent.

## Tokens

Input is divided into a series of whitespace delimited tokens.
Each of these is then processed individually. There are no
parsing words in RETRO.

Tokens may have a single character *prefix*, which RETRO will
use to decide how to process the token.

## Prefixes

Prefixes are single characters added to the start of a token
to guide the compiler. The use of these is a major way in
which RETRO differs from traditional Forth.

When a token is passed to `interpret`, RETRO first takes the
intitial character and looks to see if there is a word that
matches this. If so, it will pass the rest of the token to
that word to handle.

In a traditional Forth, the interpret process is something
like:

    get token
    is token in the dictionary?
      yes:
        is it immediate?
          yes: call the word.
          no:  are we interpreting?
               yes: call the word
               no:  compile a call to the word
      no:
        is it a number?
          yes: are we interpreting?
               yes: push the number to the stack
               no:  compile the number as a literal
          no:  report an error ("not found")

In RETRO, the interpret process is basically:

    get token
    does the first character match a `prefix:` word?
      yes: pass the token to the prefix handler
      no:  is token a word in the dictionary?
           yes: push the XT to the stack and call the
                class handler
           no:  report an error ("not found")

All of the actual logic for how to deal with tokens is moved
to the individual prefix handlers, and the logic for handling
words is moved to word class handlers.

This means that prefixes are used for a lot of things. Numbers?
Handled by a `#` prefix. Strings? Use the `'` prefix. Comments?
Use `(`. Making a new word? Use the `:` prefix.

The major prefixes are:

    | Prefix | Used For                      |
    | ------ | ----------------------------- |
    | @      | Fetch from variable           |
    | !      | Store into variable           |
    | &      | Pointer to named item         |
    | #      | Numbers                       |
    | $      | ASCII characters              |
    | '      | Strings                       |
    | (      | Comments                      |
    | :      | Define a word                 |

The individual prefixes will be covered in more detail in the
later chapters on working with different data types.

## Word Classes

Word classes are words which take a pointer and do something
with it. These are covered in detail in their own chapter,
but essentially they decide *how* to execute or compile specific
types of words.


# Additional Tools

In addition to the core `retro` binary, the `bin` directory
will contain a few other tools.

## retro

This is the main RETRO binary.

## retro-describe

This is a program that looks up entries in the Glossary.

At the command line, you can use it like:

    retro-describe s:for-each

You can pass multiple word names to it:

    retro-describe s:for-each nl d:words

## retro-embedimage

This is a program which generates a C file with the ngaImage
contents. It's used when building `retro`.

    retro-embedimage ngaImage

The output is written to stdout; redirect it as needed.

## retro-extend

This is a program which compiles code into the ngaImage.
It's used when building `retro` and when you want to make a
standalone image with custom additions.

Example command line:

    retro-extend ngaImage example/rot13.forth

Pass the image name as the first argument, and then file names
as susequent ones. Do *not* use this for things relying on I/O
apart from the basic console output as it doesn't emulate other
devices. If you need to load in things that rely on using the
optional I/O devices, see the **Advanced Builds** chapter.

## retro-muri

This is the assembler for Nga. It's used to build the initial
RETRO kernel and can be used by other tools as well.

    retro-muri retro.muri

## retro-tags and retro-locate

These tools are intended to be used together. The first tool,
`retro-tags`, will recursively scan the current directory for
RETRO source files and extract the locations of words defined
in them. These will be written to disk in a `tags` file, using
the standard ctags format.

`retro-locate` takes a word name, and returns the location(s)
where it is defined. This requires a `tags` file to be present.

Create the `tags` file:

    retro-tags

Locate a word:

    retro-locate n:square

## retro-unu

This is the literate source extraction tool for RETRO. It
is used in building `retro`.

Example usage:

    retro-unu literate/RetroForth.md

Output is written to stdout; redirect as neeeded.

# The Optional Retro Compiler

In addition to the base system, users of RETRO on Unix hosts
with ELF executables can build and use the `retro-compiler`
to generate turnkey executables.

## Requirements

- Unix host
- ELF executable support
- `objcpy` in the $PATH

## Building

   make bin/retro-compiler

## Installing

Copy `bin/retro-compiler` to somewhere in your $PATH.

## Using

`retro-compiler` takes two arguments: the source file to
compile and the name of the word to use as the main entry
point.

Example:

Given a `hello.forth`:

    ~~~
    :hello 'Hello_World! s:put nl ;
    ~~~

Use:

    retro-compiler hello.forth hello

The compiler will generate an `a.out` file which you can
then rename.

## Known Limitations

This does not provide the scripting support for command line
arguments that the standard `retro` interface offers.

A copy of `objcopy` needs to be in the path for compilation
to work.

The current working directory must be writable.

This only supports hosts using ELF executables.

The output file name is fixed to `a.out`.

RETRO(1)		    General Commands Manual		      RETRO(1)

RETRO
     retro - a modern, pragmatic forth development system

SYNOPSIS
     retro [-h] [-i] [-t] [-f filename] [-u filename] [-r filename]
	   [filename script-args]

DESCRIPTION
     RETRO is a modern, pragmatic Forth drawing influences from many sources.
     It's clean, elegant, tiny, and easy to grasp and adapt to various uses.

     retro is the main interface for interacting with Retro. It provides both
     an interactive and a scripting  model.

OPTIONS
     -h	      Display a help screen.

     -i	      Start Retro in interactive mode.

     -s	      Start Retro in interactive mode and supress the startup message.

     -t	      Run any test blocks in the loaded files.

     -f filename
	      Run any code blocks in the specified file.

     -u filename
	      Load and use the specified image file rather than the integral
	      one.

     -r filename
	      Load and run the code in the specified image file rather than
	      the integral one.

     filename script-args
	      Run code blocks in a single file. Pass script-args to the code
	      being run.

AUTHORS
     Charles Childers <crc@forthworks.com>

OpenBSD 6.4			 Setember 2019			   OpenBSD 6.4

RETRO-DESCRIBE(1)	    General Commands Manual	     RETRO-DESCRIBE(1)

RETRO-DESCRIBE
     retro-describe - a modern, pragmatic forth development system

SYNOPSIS
     retro-describe wordname [additional wordnames]

DESCRIPTION
     RETRO is a modern, pragmatic Forth drawing influences from many sources.
     It's clean, elegant, tiny, and easy to grasp and adapt to various uses.

     retro-describe is a tool for looking up the description and stack
     comments for words in the core language and extensions.  It will write
     output to stdout.

AUTHORS
     Charles Childers <crc@forthworks.com>

OpenBSD 6.4			   May 2019			   OpenBSD 6.4

RETRO-DOCUMENT(1)	    General Commands Manual	     RETRO-DOCUMENT(1)

RETRO-DOCUMENT
     retro-document - a modern, pragmatic forth development system

SYNOPSIS
     retro-document filename

DESCRIPTION
     RETRO is a modern, pragmatic Forth drawing influences from many sources.
     It's clean, elegant, tiny, and easy to grasp and adapt to various uses.

     retro-document is a tool for generating a listing of the descriptions and
     stack comments for all standard word used in a source file. It will write
     output to stdout.

AUTHORS
     Charles Childers <crc@forthworks.com>

OpenBSD 6.4			   May 2019			   OpenBSD 6.4

RETRO-EMBEDIMAGE(1)	    General Commands Manual	   RETRO-EMBEDIMAGE(1)

RETRO-EMBEDIMAGE
     retro-embedimage - a modern, pragmatic forth development system

SYNOPSIS
     retro-embedimage [filename]

DESCRIPTION
     RETRO is a modern, pragmatic Forth drawing influences from many sources.
     It's clean, elegant, tiny, and easy to grasp and adapt to various uses.

     retro-embedimage loads the specified image (or `ngaImage` from the
     current directory if none is specified). It converts this into C code
     that can be compiled for inclusion in a RETRO executable.	It will write
     the output to stdout.

AUTHORS
     Charles Childers <crc@forthworks.com>

OpenBSD 6.4			 February 2019			   OpenBSD 6.4

RETRO-EXTEND(1)		    General Commands Manual	       RETRO-EXTEND(1)

RETRO-EXTEND
     retro-extend - a modern, pragmatic forth development system

SYNOPSIS
     retro-extend image filename [filenames]

DESCRIPTION
     RETRO is a modern, pragmatic Forth drawing influences from many sources.
     It's clean, elegant, tiny, and easy to grasp and adapt to various uses.

     retro-extend is a tool to load additional code into an image file. It
     takes the name of an image file and one or more source files to load into
     the image. After completion the image file will be updated with the
     changes.


CAVETS
     retro-extend only emulates the minimal console output device. If the
     source files require additional I/O to be present, the extend process
     will likely fail to work correctly.


AUTHORS
     Charles Childers <crc@forthworks.com>

OpenBSD 6.4			 February 2019			   OpenBSD 6.4

RETRO-LOCATE(1)		    General Commands Manual	       RETRO-LOCATE(1)

RETRO-LOCATE
     retro-locate - a modern, pragmatic forth development system

SYNOPSIS
     retro-locate wordname

DESCRIPTION
     RETRO is a modern, pragmatic Forth drawing influences from many sources.
     It's clean, elegant, tiny, and easy to grasp and adapt to various uses.

     retro-locate searches the tags file generated by retro-tags for the
     desired word name. Any matches are displayed, along with the line number.

AUTHORS
     Charles Childers <crc@forthworks.com>

OpenBSD 6.6			 January 2020			   OpenBSD 6.6

RETRO-MURI(1)		    General Commands Manual		 RETRO-MURI(1)

RETRO-MURI
     retro-muri - a modern, pragmatic forth development system

SYNOPSIS
     retro-muri filename

DESCRIPTION
     RETRO is a modern, pragmatic Forth drawing influences from many sources.
     It's clean, elegant, tiny, and easy to grasp and adapt to various uses.

     retro-muri is an assembler for Nga, the virtual machine at the heart of
     Retro. It is used to build the image file containing the actual Retro
     language.

     This will extract the code blocks in the specified file and generate an
     image file named `ngaImage`.

AUTHORS
     Charles Childers <crc@forthworks.com>

OpenBSD 6.4			 February 2019			   OpenBSD 6.4

RETRO-TAGS(1)		    General Commands Manual		 RETRO-TAGS(1)

RETRO-TAGS
     retro-tags - a modern, pragmatic forth development system

SYNOPSIS
     retro-tags

DESCRIPTION
     RETRO is a modern, pragmatic Forth drawing influences from many sources.
     It's clean, elegant, tiny, and easy to grasp and adapt to various uses.

     retro-tags is a tool for extracting code from fenced blocks in literate
     sources and generating a tags file compatible with ctags.

AUTHORS
     Charles Childers <crc@forthworks.com>

OpenBSD 6.4			  August 2019			   OpenBSD 6.4

# Naming Conventions

Word names in RETRO generally follow the following conventions.

## General Guidelines

* Readability is important
* Be consistent
* Don't use a prefix as the first character of a name
* Don't use underscores in word names
* Use short names for indices
* Word names start with a `-` for "not"
* Words returning a flag end in ?

## Typical Format

The word names will generally follow a form like:

    [namespace:]name

The `namespace:` is optional, but recommended for consistency
with the rest of the system and to make it easier to identify
related words.

## Case

Word names are lowercase, with a dash (-) for compound names.

    hello
    drop-pair
    s:for-each

Variables use TitleCase, with no dash between compound names.

    Base
    Heap
    StringBuffers

Constants are UPPERCASE, with a dash (-) for compound names.

    TRUE
    FALSE
    f:PI
    MAX-STRING-LENGTH

## Namespaces

Words are grouped into broad namespaces by attaching a short
prefix string to the start of a name.

The common namespaces are:

    | Prefix  | Contains                                               |
    | ------- | ------------------------------------------------------ |
    | a:      | Words operating on simple arrays                       |
    | ASCII:  | ASCII character constants for control characters       |
    | buffer: | Words for operating on a simple linear LIFO buffer     |
    | c:      | Words for operating on ASCII character data            |
    | class:  | Contains class handlers for words                      |
    | d:      | Words operating on the Dictionary                      |
    | err:    | Words for handling errors                              |
    | io:     | General I/O words                                      |
    | n:      | Words operating on numeric data                        |
    | prefix: | Contains prefix handlers                               |
    | s:      | Words operating on string data                         |
    | v:      | Words operating on variables                           |
    | file:   | File I/O words                                         |
    | f:      | Floating Point words                                   |
    | unix:   | Unix system call words                                 |

## Tips

### Don't Start Names With Prefix Characters

Avoid using a prefix as the first character of a word name. RETRO
will look for prefixes first, this will prevent direct use of
the work in question.

To find a list of prefix characters, do:

    'prefix: d:words-with

### Don't Use Underscores

Underscores in strings are replaced by spaces. This is problematic,
especially with variables. Consider:

    'test_name var
    #188 !test_name

In this, the string for the name is converted to "test name". The
store in the second line will not add the space, so resolves to an
incorrect address.

I personally recommend avoiding the use of underscores in any word
names.

# The Return Stack

RETRO has two stacks. The primary one is used to pass data
beween words. The second one primarily holds return addresses.

Each time a word is called, the next address is pushed to
the return stack.

# Stack Diagrams

Most words in RETRO have a stack comment. These look like:

    (-)
    (nn-n)

As with all comments, a stack comment begins with `(` and
should end with a `)`. There are two parts to the comment.
On the left side of the `-` is what the word *consumes*. On
the right is what it *leaves*.

RETRO uses a short notation, with one character per value
taken or left. In general, the following symbols represent
certain types of values.

    | Notation            | Represents              |
    | ------------------- | ----------------------- |
    | b, n, m, o, x, y, z | generic numeric values  |
    | s                   | string                  |
    | v                   | variable                |
    | p, a                | pointers                |
    | q                   | quotation               |
    | d                   | dictionary header       |
    | f                   | `TRUE` or `FALSE` flag. |

In the case of something like `(xyz-m)`, RETRO expects z to be
on the top of the stack, with y below it and x below the y
value. And after execution, a single value (m) will be left on
the stack.

Words with no stack effect have a comment of (-)

# Working With Arrays

RETRO offers a number of words for operating on statically sized
arrays.

## Namespace

The words operating on arrays are kept in an `a:` namespace.

## Creating Arrays

The easiest way to create an array is to wrap the values in a
`{` and `}` pair:

```
{ #1 #2 #3 #4 }
{ 'this 'is 'an 'array 'of 'strings }
{ 'this 'is 'a 'mixed 'array #1 #2 #3 }
```

You can also make an array from a quotation which returns
values and the number of values to store in the a:

```
[ #1 #2 #3  #3 ] a:counted-results
[ #1 #2 #3  #3 ] a:make
```

## Accessing Elements

You can access a specific value with `a:th` and `fetch` or
`store`:

```
{ #1 #2 #3 #4 } #3 a:th fetch
```

## Find The Length

Use `a:length` to find the size of the array.

```
{ #1 #2 #3 #4 } a:length
```

## Duplicate

Use `a:dup` to make a copy of an a:

```
{ #1 #2 #3 #4 } a:dup
```

## Filtering

RETRO provides `a:filter` which extracts matching values
from an array. This is used like:

```
{ #1 #2 #3 #4 #5 #6 #7 #8 } [ n:even? ] a:filter
```

The quote will be passed each value in the array and should
return TRUE or FALSE. Values that lead to TRUE will be collected
into a new array.

## Mapping

`a:map` applies a quotation to each item in an array and
constructs a new array from the returned values.

Example:

```
{ #1 #2 #3 } [ #10 * ] a:map
```

## Reduce

`a:reduce` takes an array, a starting value, and a quote. It
executes the quote once for each item in the array, passing the
item and the value to the quote. The quote should consume both
and return a new value.

```
{ #1 #2 #3 } #0 [ + ] a:reduce
```

## Search

RETRO provides `a:contains?` and `a:contains-string?`
to search an array for a value (either a number or string) and
return either TRUE or FALSE.

```
#100  { #1 #2 #3 } a:contains?
'test { 'abc 'def 'test 'ghi } a:contains-string?
```

## Implementation

In memory, an array is a count followed by the values. As an
example, if you have an array:

    { #10 #20 #30 }

In memory this would be setup as:

    | Offset | Value |
    | ------ | ----- |
    | 000    | 3     |
    | 001    | 10    |
    | 002    | 20    |
    | 003    | 30    |

You can construct one on the fly by keeping a pointer to
`here` and using `,` to place the values. E.g.,

    here [ #3 , #10 , #20 , #30 , ] dip

An example of this can be seen in this excerpt from an example
(*example/Primes.forth*):

    :create-set (-a) 
        here #3000 , #2 #3002 [ dup , n:inc ] times drop ;

# Working With Assembly Language

RETRO runs on a virtual machine called Nga. It provides a
standard assembler for this called *Muri*.

Muri is a simple, multipass model that's not fancy, but
suffices for RETRO's needs.

## Assembling A Standalone File

A small example (*test.muri*)

    ~~~
    i liju....
    r main
    : c:put
    i liiire..
    i 0
    : main
    i lilica..
    d 97
    i liju....
    r main
    ~~~

Assembling it:

    retro-muri test.muri

So breaking down: Muri extracts the assembly code blocks to
assemble, then proceeds to do the assembly. Each source line
starts with a directive, followed by a space, and then ending
with a value.

The directives are:

    :    value is a label
    i    value is an instruction bundle
    d    value is a numeric value
    r    value is a reference
    s    value is a string to inline

Instructions for Nga are provided as bundles. Each memory
location can store up to four instructions. And each instruction
gets a two character identifier.

From the list of instructions:

    0 nop   5 push  10 ret   15 fetch 20 div   25 zret
    1 lit   6 pop   11 eq    16 store 21 and   26 halt
    2 dup   7 jump  12 neq   17 add   22 or    27 ienum
    3 drop  8 call  13 lt    18 sub   23 xor   28 iquery
    4 swap  9 ccall 14 gt    19 mul   24 shift 29 iinvoke

This reduces to:

    0 ..    5 pu    10 re    15 fe    20 di    25 zr
    1 li    6 po    11 eq    16 st    21 an    26 ha
    2 du    7 ju    12 ne    17 ad    22 or    27 ie
    3 dr    8 ca    13 lt    18 su    23 xo    28 iq
    4 sw    9 cc    14 gt    19 mu    24 sh    29 ii

Most are just the first two letters of the instruction name. I
use `..` instead of `no` for `NOP`, and the first letter of
each I/O instruction name. So a bundle may look like:

    dumure..

(This would correspond to `dup multiply return nop`).

## Runtime Assembler

RETRO also has a runtime variation of Muri that can be used
when you need to generate more optimal code. So one can write:

    :n:square dup * ;

Or:

    :n:square \dumure.. ;

The second one will be faster, as the entire definition is one
bundle, which reduces memory reads and decoding by 2/3.

Doing this is less readable, so I only recommend doing so after
you have finalized working RETRO level code and determined the
best places to optimize.

The runtime assembler has the following directives:

    i    value is an instruction bundle
    d    value is a numeric value
    r    value is a reference

Additionally, in the runtime assembler, these are reversed:

    'dudumu.. i

Instead of:

    i dudumu..

The runtime assembler also provides three prefixes for use in
inlining machine code into a definition. These are:

     \    Treat token as an assembly sequence
     `    Treat token as a numeric value
     ^    Treat token as a reference

E.g., instead of doing something like:

    :n:square as{ 'dumu.... i }as ;
    :test as{ 'lilica.... i #22 d 'n:square r }as ;

Just write:

    :n:square \dumu.... ;
    :test     \lilica.. `22 ^n:square ; 

# Working With a Buffer

RETRO provides words for operating on a linear memory area.
This can be useful in building strings or custom data
structures.

## Namespace

Words operating on the buffer are kept in the `buffer:`
namespace.

## Implementation

A buffer is a linear sequence of memory. The buffer words
provide a means of incrementally storing and retrieving
values from it.

The buffer words keep track of the start and end of the
buffer. They also ensure that an `ASCII:NULL` is written
after the last value, which make using them for string
data easy.

## Limitations

Only one buffer can be active at a time. RETRO provides a
`buffer:preserve` combinator to allow using a second one
before returning to the prior one.

## Set The Active Buffer

To set a buffer as the active one use `buffer:set`. This takes
an address.

The buffer will be assumed to be empty. The inital value will
be set to ASCII:NULL.

## Add Value

Use `buffer:add` to append a value to the buffer. This takes
a single value and will also add an ASCII:NULL after the end
of the buffer.

## Fetch Last Value

To return the last value in the buffer you can use `buffer:get`.
This removes the value and sets an ASCII:NULL in the memory
location the returned value occupied.

## Get Data About The Buffer

RETRO provides `buffer:start` to get the initial address in
the buffer, `buffer:end` to get the last address (ignoring the
ASCII:NULL), and `buffer:size` to return the number of values
in the buffer.

## Reset

You can reset a buffer to the empty state using `buffer:empty`.

## Example

To begin, create a memory region to use as a buffer.

```
'Test d:create #1025 allot
```

Then you can set this as the current buffer:

```
&Test buffer:set
```

When a buffer is set, the vocabulary sets an internal
index to the first address in it. This will be
incremented when you add data and decremented when you
remove data.

Let's add some stuff using `buffer:add`:

```
#100 buffer:add
#200 buffer:add
#300 buffer:add
```

And then retreive the values:

```
buffer:get n:put nl
buffer:get n:put nl
buffer:get n:put nl
```

You can remove all values using `buffer:empty`:

```
#100 buffer:add
#200 buffer:add
#300 buffer:add
buffer:empty
```

And ask the buffer how many items it contains:

```
buffer:size n:put nl
#100 buffer:add
#200 buffer:add
#300 buffer:add
buffer:size n:put nl
buffer:empty
```

The other functions are `buffer:start`, which returns
the address of the buffer, `buffer:end`, which returns
the address of the last value, and `buffer:preserve`.
The first is easy to demo:

```
buffer:start Test eq? n:put nl
```

The last one is useful. Only one buffer is ever active
at a given time. The `buffer:preserve` combinator lets
you execute a word, saving and restoring the current
buffer indexes. So the word could assign and use a new
buffer and this will reset the previous one after
control returns.

There are a few notes that need to be considered. The
preserve combinator saves the start and current index
but *not* the contents. If the word you call uses the
same buffer, the contents will remain altered.

Finally, the buffer words have one interesting trait:
they store an ASCII NULL after adding each item to the
buffer. This lets one use them to build strings easily.

```
Test buffer:set
$h buffer:add
$e buffer:add
$l buffer:add
$l buffer:add
$o buffer:add
$, buffer:add
#32 buffer:add
$w buffer:add
$o buffer:add
$r buffer:add
$l buffer:add
$d buffer:add
buffer:start s:put nl
```

# Working With Characters

RETRO provides words for working with ASCII characters.

## Prefix

Character constants are returned using the `$` prefix.

## Namespace

Words operating on characters are in the `c:` namespace.

## Classification

RETRO provides a number of words to determine if a character
fits into predefined groups.

The primary words for this are:

* `c:consonant?`
* `c:digit?`
* `c:letter?`
* `c:lowercase?`
* `c:uppercase?`
* `c:visible?`
* `c:vowel?`
* `c:whitespace?`

There are also corresponding "not" forms:

* `c:-consonant?`
* `c:-digit?`
* `c:-lowercase?`
* `c:-uppercase?`
* `c:-visible?`
* `c:-vowel?`
* `c:-whitespace?`

All of these take a character and return either a `TRUE` or
`FALSE` flag.

## Conversions

A few words are provided to convert case. Each takes a character
and returns the modified character.

* `c:to-lower`
* `c:to-number`
* `c:to-upper`
* `c:toggle-case`

RETRO also has `c:to-string`, which takes a character and
creates a new temporary string with the character.

## I/O

Characters can be displayed using `c:put`.

```
$a c:put
```

With the default system on BSD, Linux, and macOS (and other
Unix style hosts), `c:get` is provided to read input. This
may be buffered, depending on the host.

# Defining Words

Words are named functions. To start a word, preceed it's name
with a colon. Follow this by the definition, and end with a
semicolon.

E.g.,

    :do-nothing ;
    :square dup * ;

# Working With The Dictionary

The Dictionary is a linked list containing the dictionary
headers.

## Namespace

Words operating on the dictionary are in the `d:` namespace.

## Variables

`Dictionary` is a variable holding a pointer to the most recent
header.

## Header Structure

Each entry follows the following structure:

    Offset   Contains
    ------   ---------------------------
    0000     Link to Prior Header
    0001     Link to XT
    0002     Link to Class Handler
    0003+    Word name (null terminated)

RETRO provides words for accessing the fields in a portable
manner. It's recommended to use these to allow for future
revision of the header structure.

## Accessing Fields

Given a pointer to a header, you can use `d:xt`, `d:class`,
and `d:name` to access the address of each specific field.
There is no `d:link`, as the link will always be the first
field.

## Shortcuts For The Latest Header

RETRO provides several words for operating on the most recent
header.

`d:last` returns a pointer to the latest header. `d:last.xt`
will give the contents of the `d:xt` field for the latest
header. There are also `d:last.class` and `d:last.name`.

## Adding Headers

Two words exist for making new headers. The easy one is
`d:create`. This takes a string for the name and makes a
new header with the class set to `class:data` and the XT
field pointing to `here`.

Example:

```
'Base d:create
```

The other is `d:add-header`. This takes a string, a pointer
to the class handler, and a pointer for the XT field and
builds a new header using these.

Example:

```
'Base &class:data #10000 d:add-header
```

## Searching

RETRO provides two words for searching the dictionary.

`d:lookup` takes a string and tries to find it in the
dictionary. It will return a pointer to the dictionary header
or a value of zero if the word was not found.

`d:lookup-xt` takes a pointer and will return the dictionary
header that has this as the `d:xt` field, or zero if no match
is found.

## Iteration

You can use the `d:for-each` combinator to iterate over all
entries in the dictionary. For instance, to display the names
of all words:

```
[ d:name s:put sp ] d:for-each
```

For each entry, this combinator will push a pointer to the
entry to the stack and call the quotation.

## Listing Words

Most Forth systems provide WORDS for listing the names of all
words in the dictionary. RETRO does as well, but this is named
`d:words`.

This isn't super useful as looking through several hundred
names is annoying. RETRO also provides `d:words-with` to help
in filtering the results.

Example:

```
'class: d:words-with
```


# Working With Floating Point

Some RETRO systems include support for floating point numbers.
When present, this is built over the system `libm` using the
C `double` type.

Floating point values are typically 64 bit IEEE 754 double
precision (1 bit for the sign, 11 bits for the exponent, and
the remaining 52 bits for the value), i.e. 15 decimal digits
of precision.

## Prefix

Floating point numbers start with a `.`

Examples:

   Token    Value
    .1       1.0
    .0.5     0.5
    .-.4    -0.4
    .1.3     1.3

## Namespace

Floating point words are in the `f:` namespace. There is also
a related `e:` namespace for *encoded values*, which allows
storing of floats in standard memory.

## Operation

Floating point values exist on a separate stack, and are bigger
than the standard memory cells, so can not be directly stored
and fetched from memory.

The floating point system also provides an alternate stack that
can be used to temporarily store values.

The following words exist for arranging values on the floating
point stack. These are direct analogs to the non-prefiexd words
for dealing with the data stack.

- `f:nip`
- `f:over`
- `f:depth`
- `f:drop`
- `f:drop-pair`
- `f:dup`
- `f:dup-pair`
- `f:dump-stack`
- `f:tuck`
- `f:swap`
- `f:rot`

For the secondary floating point stack, the following words are
provided:

- `f:push`
- `f:pop`
- `f:adepth`
- `f:dump-astack`

## Constants

    | Name     | Returns           |
    | -------- | ----------------- |
    | `f:E`    | Euler's number    |
    | `f:-INF` | Negative infinity |
    | `f:INF`  | Positive infinity |
    | `f:NAN`  | Not a Number      |
    | `f:PI`   | PI                |

## Comparisons

The basic set of comparators are the same as those for
operating on integers. These are:

- `f:-eq?`
- `f:between?`
- `f:eq?`
- `f:gt?`
- `f:lt?`
- `f:negative?`
- `f:positive?`
- `f:case`

There are also a few additions for comparing to special values
like infinity and NaN.

- `f:-inf?`
- `f:inf?`
- `f:nan?`

## Basic Math

- `f:*`
- `f:+`
- `f:-`
- `f:/`
- `f:abs`
- `f:floor`
- `f:inc`
- `f:limit`
- `f:max`
- `f:min`
- `f:negate`
- `f:power`
- `f:ceiling`
- `f:dec`
- `f:log`
- `f:sqrt`
- `f:square`
- `f:round`
- `f:sign`
- `f:signed-sqrt`
- `f:signed-square`

## Geometry

RETRO provides a small number of words for doing geometric
related calculations.

| Word     | Returns      |
| -------- | ------------ |
| `f:acos` | arc cosine   |
| `f:asin` | arc sine     |
| `f:atan` | arc tangent  |
| `f:cos`  | cosine       |
| `f:sin`  | sine         |
| `f:tan`  | tangent      |

## Storage and Retrieval

By leveraging the encoded value functions, RETRO is able to
allow storage of floating point values in memory. This does
have a tradeoff in accuracy as the memory cells are considerably
smaller than a full floating point size.

You can use `f:fetch` to fetch a floating point value and
`f:store` to store one.

If you need more precision, try Kiyoshi Yoneda's FloatVar
example (`example/FloatVar.forth`), which includes words to
store and retrieve values using multiple cells.

- `f:to-number`
- `f:to-string`

## I/O

The floating point vocabulary has a single I/O word, `f:put`,
for the display of floating point numbers.

## Encoded Values

RETRO provides a means of encoding and decoding floating point
values into standard integer cells. This is based on the paper
"Encoding floating point values to shorter integers" by Kiyoshi
Yoneda and Charles Childers.

- `f:E1`
- `f:to-e`
- `e:-INF`
- `e:-inf?`
- `e:INF`
- `e:MAX`
- `e:MIN`
- `e:NAN`
- `e:clip`
- `e:inf?`
- `e:max?`
- `e:min?`
- `e:n?`
- `e:nan?`
- `e:put`
- `e:to-f`
- `e:zero?`

# Working With Files

On Unix and Windows systems RETRO provides a set of words for
working with files. As a pragmatic choice these are mostly
modeled after the file functions in libc.

The file words are in the `file:` namespace.

## File Access Modes

You can open a file for various operations. The functionality
allowed depends on the file access mode. Valid modes in RETRO
are:

    file:A    Open for appending; file pointer set to end of file
    file:R    Open for reading; file pointer set to start of file
    file:R+   Open for reading and writing
    file:W    Open for writing

## Opening A File

To open a file, pass the file name and a file mode to `file:open`.

    '/etc/motd file:R file:open

On a successful open this will return a file handle greater than
zero.

Additionally, RETRO provides a few other forms for opening files.

To open a file for reading:

    '/etc/motd file:open-for-reading

This will return the size of the file (as NOS) and the file handle
(as TOS).

To open a file for writing:

    '/tmp/test file:open-for-writing

This returns the file handle.

To open a file for append operations:

    '/tmp/test file:open-for-append

As with `file:open-for-reading`, this returns both the size of
the file and the file handle.

## Closing A File

To close a file, pass the file handle to `file:close`.

    '/etc/motd file:A file:open file:close

## Reading From A File

To read a byte from an open file, pass the file handle to the
`file:read` word.

    @FID file:read n:put

To read a line from a file, pass the file handle to the word
`file:read-line`.

    @FID file:read-line s:put

The line is read into a temporary string buffer. Move the
text to a safe place if you aren't using it quickly or if
the length of the line is bigger than the size of a temporary
string.

## Writing To A File

To write a byte to a file, pass it and the file handle to
`file:write`.

    $h @FID file:write
    $e @FID file:write
    $l @FID file:write
    $l @FID file:write
    $o @FID file:write

Though cells are 32 or 64 bits in size, only the byte value will
be written to the file.

## Deleting Files

You can delete a file by passing the file name to `file:delete`.

    /tmp/test file:delete

## Check For File Existance

Use `file:exists?` to detect the existance of a file. Pass it a
file name and it will return `TRUE` if existing or `FALSE` if
it does not.

    '/etc/motd file:exists?

This will also return `TRUE` if the filename is a directory.

## Flush Caches

Use `file:flush` to flush the system caches for a file. Pass a
file handle to this.

    @FID file:flush

## Seek A Position Within A File

You can use `file:seek` to move the internal file pointer
for a given file. Pass this the new location and a file.

    #100 @FID file:seek

The location for the file pointer is a fixed offset from the
start of the file, not a relative offset.

## Get The Current Position Within A File

To find the current value of the file pointer within a file
just pass the file handle to `file:tell`.

    @FID file:tell

This returns a number that is the number of bytes into the file
that the file pointer is currently at.

## Determine The Size Of A File

Use `file:size` to return the size of a file. Pass this a file
handle and it will return the size of a file, or 0 if empty. If
the file is a directory, it returns -1.

    @FID file:size

## Reading An Entire File

If you want to read an entire file into memory you can use
`file:slurp`. This takes the starting address of a memory
region and the name of the file.

    here '/etc/motd file:slurp

Take care that the memory buffer is large enough for the file
being read or you will run into problems.

## Writing A String To A File

If you have a string that you want to write to a file, replacing
any existing contents, you can use `file:spew`. This takes the
string to write and a file name.

    'hello_world '/tmp/test.txt file:spew

## Iterating Over A File, Line By Line

You can easily iterate over each line in a file using the word
`file:for-each-line`. This will take a file name and a quote,
read each line into a temporary string, then pass this string to
the quote.

    '/etc/motd [ s:put nl ] file:for-each-line

# Loops

RETRO provides several words for creating loops.

## Unconditional Loops

An unconditional loop begins with `repeat` and ends with `again`.

    :test repeat #1 n:put sp again ;
    test

Unconditional loops must be inside a definition or quote. To exit
one of these, use `0;`, `-if;` or `if;`.

    :test #100 repeat 0; dup n:put sp n:dec again ;
    test

    :test #100 repeat dup #50 eq? [ 'done! s:put nl ] if; n:dec again ;
    test

You can also achieve this via recursion:

    :test 0; dup n:put sp n:dec test ;
    #100 test

Be careful with recursion as the virtual machine will have a limited
amount of space for the address stack and recursing too many times
can cause a stack overflow.

## Conditional Loops

There are two conditional looping combinators: `while` and `until`.
Both take a quote and execute it, checking a returned flag to decide
when to stop running.

    #0 [ dup n:put sp n:inc dup #10 eq? ] until
    #10 [ dup n:put sp n:dec dup n:-zero? ] while

## Counted Loops

There are two combinators for counted loops. These are `times` and
`indexed-times`.

    #0 #10 [ dup n:put sp n:inc ] times nl
    #10 [ I n:put sp ] indexed-times

The `indexed-times` provides an index via the `I`, `J`, and
`K` words. `I` will be the index of the current loop, with `J` and
`K` being the indexes of the next two older loops.

The loop indexes can be accessed outside the loop body:

    :display I n:square n:put sp ;
    :squares [ display ] indexed-times nl ;
    #100 squares

## Tradeoffs

The unconditional loop form is more efficient as it's just a
simple jump operation. The `times` counted loops are a little
slower, but can be cleaner and more readable in many cases. The
`indexed-times` form is significantly slower than the other
two forms.

# Working With Numbers

Numbers in RETRO are signed integers.

## Token Prefix

All numbers start with a `#` prefix.

## Namespace

Most words operating on numbers are in the `n:` namespace.

## Range of Values

A default RETRO system with 32 bit cells provides a range of
-2,147,483,648 to 2,147,483,647. For 64 bit systems, the range
will be -9,223,372,036,854,775,807 to 9,223,372,036,854,775,806.

You can check the range your VM and image support using:

    n:MIN
    n:MAX

These will return the limits for your system.

## Comparisons

RETRO provides a number of comparison words for numeric values.

The basic comparators are:

    -eq?
    eq?
    lt?
    lteq?
    gt?
    gteq?

Additionally RETRO also provides:

    n:-zero?
    n:between?
    n:even?
    n:negative?
    n:odd?
    n:positive?
    n:strictly-positive?
    n:zero?

## Basic Operations

    +
    -
    *
    /
    mod
    /mod
    n:abs
    n:dec
    n:inc
    n:limit
    n:max
    n:min
    n:negate
    n:pow
    n:sqrt
    n:square

## Conversions

You can convert a number to a string with `n:to-string` or
to a floating point value with `n:to-float`.

    #123 n:to-float  f:put

    #123 n:to-string s:put

## Display

To display a number, use `n:put`.

    #123 n:put

# Working With Pointers

## Prefix

Pointers are returned by the `&` prefix.

## Examples

```
'Base var
&Base fetch
#10 &Base store

#10 &n:inc call
```

## Notes

The use of `&` to get a pointer to a data structure (with a
word class of `class:data`) is not required. I like to use it
anyway as it makes my intent a little clearer.

Pointers are useful with combinators. Consider:

```
:abs dup n:negative? [ n:negate ] if ;
```

Since the target quote body is a single word, it is more
efficient to use a pointer instead:

```
:abs dup n:negative? &n:negate if ;
```

The advantages are speed (saves a level of call/return by
avoiding the quotation) and size (for the same reason).
This may be less readable though, so consider the balance
of performance to readability when using this approach.

# Quotations

Quotes are anonymous functions. RETRO uses these as the basis for
executable flow control and combinatorial logic.

## Using Quotations

To make a quotation, surround the code with square brackets. E.g.,

    #1 #2 eq? [ 'No_match s:put nl ] -if

Quotes can be nested:

    [ #3 [ #4 ] dip ] call

After creation, a pointer to the quotation is left on the stack
(or is compiled into the current definition).

## Combinators

Words operating on quotations are called combinators; these are
discussed in *Using Combinators*.

## Implementation

A quotation is compiled as:

    ... code before quotation ...
    i liju....                    (if_compiling_only)
    d address after quotation     (if_compiling_only)
    ... code for quotation
    i re......                    (this_is_where_the_quote_ends)
    i li......
    d address of code for quotation
    ... code after quotation ....

## Other Notes

Quotations are used heavily in RETRO. They give the source a
feel that's different from traditional Forth, and allow for
a more consistent syntax.

For instance, in a traditional Forth, you might have some
conditionals:

    IF ... THEN
    IF ... ELSE ... THEN
    IF ... EXIT THEN

RETRO uses conditional combinators for these:

    [ ... ] if
    [ ... ] [ ... ] choose
    [ ... ] if;

Or loops:

    FOR ... NEXT

Is replaced by:

    [ ... ] times

This can also extend to stack flow. Sequences like:

    >R ... >R
    DUP >R ... >R

Become:

    [ ... ] dip
    [ ... ] sip

And forms like:

    1 2 3 * swap 3 * swap

Can be replaced with a combinator like:

    #1 #2 [ #3 * ] bi@

While there is a different set of words to learn, I find that
overall there's less noise from low level stack shuffling words
and the added consistency with regards to overall syntax has
been nice as I was never fond of the multiple forms that existed
in traditional Forth.

# Sockets

On Unix hosts, RETRO provides an optional set of words for using
network sockets.

## Create a Socket

To create a new socket, just run:

    socket:create

This will return a socket handle.

## Bind To A Port

To bind to a port, pass the port number and socket handle
to `socket:bind`. The port should be a string. This will return
0 if successful, -1 if not successful, and an error code.

    '9998 @Sock socket:bind

## Configure To Allow Incoming Connections

To prepare a socket for incoming connections use socket:listen. This
will take a backlog count and a socket handle. It returns a flag
(0 success, -1 failed) and an error code.

    #3 @Sock socket:listen

## Accept Connections

To accept connections pass the socket handle to `socket:accept`.
This returns a new socket for the connection and an error code.

    @Sock socket:accept

## Make A Connection

To connect to a server using the socket:

    'forth.works '70 socket:configure
    @Sock socket:connect

`socket:connect` will return a status code and an error code.

## Writing To A Socket

To write a string to a socket, use `socket:send`. This will
take a string and a socket handle and will return the number
of bytes sent and an error code.

    'test @Sock socket:send

## Reading From A Socket

To read data from a socket pass an address, a maximum number of
bytes, and the socket handle to `socket:recv`. This will return
the number of bytes received and an error code. The bytes will
be stored in memory starting at the specified address.

    here #1024 @Sock socket:recv

## Close a Socket

To close a socket, pass the socket handle to `socket:close`.

    @Socket socket:close

# Unix Scripting

RETRO on Unix hosts is designed to play well with scripting. 

## Shebang

To run an entire program directly, start the file with the
standard shebang and make the file executable:

    #!/usr/bin/env retro

This requires the `retro` binary to be in your path.

## Arguments

RETRO provides several words in the `script:` namespace for accessing
command line arguments.

The number of arguments can be accessed via `script:arguments`. This
will return a number with the arguments, other than the script
name.

    script:arguments '%n_arguments_passed\n s:format s:put

To retreive an argument, pass the argument number to `script:get-argument`:

    script:arguments [ I script:get-argument s:put nl ] indexed-times 

And to get the name of the script, use `script:name`.

    script:name s:put

## Mixing

With use of the Unu literate format, it's possible to mix both
shell and RETRO code into a single script. As an example, this
is a bit of shell that runs itself via retro for each .retro
file in the current directory tree:

    #!/bin/sh

    # shell part
    find . -name '*.retro' -print0 | xargs -0 -n 1 retro $0
    exit

    # retro part

    This will scan a source file and do something with it:

    ~~~
    ... do stuff ...
    ~~~

# Working With Strings

Strings in RETRO are NULL terminated sequences of values
representing characters. Being NULL terminated, they can't
contain a NULL (ASCII 0).

The character words in RETRO are built around ASCII, but
strings can contain UTF8 encoded data if the host platform
allows. Words like `s:length` will return the number of bytes,
not the number of logical characters in this case.

## Prefix

Strings begin with a single `'`.

    'Hello
    'This_is_a_string
    'This_is_a_much_longer_string_12345_67890_!!!

RETRO will replace spaces with underscores. If you need both
spaces and underscores in a string, escape the underscores and
use `s:format`:

    'This_has_spaces_and_under\_scored_words. s:format

## Namespace

Words operating on strings are in the `s:` namespace.

## Lifetime

At the interpreter, strings get allocated in a rotating buffer.
This is used by the words operating on strings, so if you need
to keep them around, use `s:keep` or `s:copy` to move them to
more permanent storage.

In a definition, the string is compiled inline and so is in
permanent memory.

You can manually manage the string lifetime by using `s:keep`
to place it into permanent memory or `s:temp` to copy it to
the rotating buffer.

## Mutability

Strings are mutable. If you need to ensure that a string is
not altered, make a copy before operating on it or see the
individual glossary entries for notes on words that may do
this automatically.

## Searching

RETRO provides four words for searching within a string.

* `s:contains-char?` 
* `s:contains-string?`
* `s:index-of`
* `s:index-of-string`

## Comparisons

* `s:eq?`
* `s:case`

## Extraction

To obtain a new string containing the first `n` characters from
a source string, use `s:left`:

    'Hello_World #5 s:left

To obtain a new string containing the last `n` characters from
a source string, use `s:right`:

    'Hello_World #5 s:right

If you need to extract data from the middle of the string, use
`s:substr`. This takes a string, the offset of the first
character, and the number of characters to extract.

    'Hello_World #3 #5 s:substr

## Joining

You can use `s:append` or `s:prepend` to merge two strings.

    'First 'Second s:append
    'Second 'First s:prepend

## Tokenization

* `s:tokenize`
* `s:tokenize-on-string`
* `s:split`
* `s:split-on-string`

## Conversions

To convert the case of a string, RETRO provides `s:to-lower`
and `s:to-upper`.

`s:to-number` is provided to convert a string to an integer
value. This has a few limitations:

- only supports decimal
- non-numeric characters will result in incorrect values

## Cleanup

RETRO provides a handful of words for cleaning up strings.

`s:chop` will remove the last character from a string. This
is done by replacing it with an ASCII:NULL.

`s:trim` removes leading and trailing whitespace from a string.
For more control, there is also `s:trim-left` and `s:trim-right`
which let you trim just the leading or trailing end as desired.

## Combinators

* `s:for-each`
* `s:filter`
* `s:map`

## Other

* `s:evaluate`
* `s:copy`
* `s:reverse`
* `s:hash`
* `s:length`
* `s:replace`
* `s:format`
* `s:empty`

## Controlling The Temporary Buffers

As dicussed in the Lifetime subsection, temporary strings are
allocated in a rotating buffer. The details of this can be
altered by updating two variables.

    | Variable      | Holds                                    |
    | ------------- | ---------------------------------------- |
    | TempStrings   | The number of temporary strings          |
    | TempStringMax | The maximum length of a temporary string |

For example, to increase the number of temporary strings to
48:

    #48 !TempStrings

The defaults are:

    | Variable      | Default |
    | ------------- | ------- |
    | TempStrings   | 32      |
    | TempStringMax | 512     |

It's also important to note that altering these will affect
the memory map for all temporary buffers. Do not use anything
already in the buffers after updating these or you will risk
data corruption and possible crashes.

# Using Combinators

A combinator is a function that consumes functions as input.
They are used heavily by the RETRO system.

## Types of Combinators

Combinators are divided into three primary types: compositional,
execution flow, and data flow.

## Compositional

A compositional combinator takes elements from the stack and
returns a new quote.

`curry` takes a value and a quote and returns a new quote
applying the specified quote to the specified value. As an
example,

```
:acc (n-)  here swap , [ dup v:inc fetch ] curry ;
```

This would create an accumulator function, which takes an
initial value and returns a quote that will increase the
accumulator by 1 each time it is invoked. It will also return
the latest value. So:

```
#10 acc
dup call n:put
dup call n:put
dup call n:put
```

## Execution Flow

Combinators of this type execute other functions.

### Fundamental

`call` takes a quote and executes it immediately.

```
[ #1 n:put ] call
&words call
```

### Conditionals

RETRO provides three primary combinators for use with
conditional execution of quotes. These are `choose`, `if`,
and `-if`.

`choose` takes a flag and two quotes from the stack. If the
flag is true, the first quote is executed. If false, the
second quote is executed.

```
#-1 [ 'true s:put ] [ 'false s:put ] choose
 #0 [ 'true s:put ] [ 'false s:put ] choose
```

`if` takes a flag and one quote from the stack. If the flag is
true, the quote is executed. If false, the quote is discarded.

```
#-1 [ 'true s:put ] if
 #0 [ 'true s:put ] if
```

`-if` takes a flag and one quote from the stack. If the flag is
false, the quote is executed. If true, the quote is discarded.

```
#-1 [ 'false s:put ] -if
 #0 [ 'false s:put ] -if
```

RETRO also provides `case` and `s:case` for use when you have
multiple values to check against. This is similar to a `switch`
in C.

`case` takes two numbers and a quote. The initial value is
compared to the second one. If they match, the quote is
executed. If false, the quote is discarded and the initial
value is left on the stack.

Additionally, if the first value was matched, `case` will exit
the calling function, but if false, it returns to the calling
function.

`s:case` works the same way, but for strings instead of simple
values.

```
:test (n-)
  #1 [ 'Yes s:put ] case
  #2 [ 'No  s:put ] case
  drop 'No idea s:put ;
```

### Looping

Several combinators are available for handling various looping
constructs.

`while` takes a quote from the stack and executes it repeatedly
as long as the quote returns a true flag on the stack. This flag
must be well formed and equal -1 or 0.

```
#10 [ dup n:put sp n:dec dup 0 -eq? ] while
```

`times` takes a count and quote from the stack. The quote will
be executed the number of times specified. No indexes are pushed
to the stack.

```
#1 #10 [ dup n:put sp n:inc ] times drop
```

There is also a `indexed-times` variation that provides
access to the loop index (via `I`) and parent loop indexes
(via `J` and `K`).

```
#10 [ I n:put sp ] indexed-times
```

## Data Flow

These combinators exist to simplify stack usage in various
circumstances.

### Preserving

Preserving combinators execute code while preserving portions
of the data stack.

`dip` takes a value and a quote, moves the value off the main
stack temporarily, executes the quote, and then restores the
value.

```
#10 #20 [ n:inc ] dip
```

Would yield the following on the stack:

```
11 20
```

`sip` is similar to `dip`, but leaves a copy of the original
value on the stack during execution of the quote. So:

```
#10 [ n:inc ] sip
```

Leaves us with:

```
11 10
```

### Cleave

Cleave combinators apply multiple quotations to a single value
or set of values.

`bi` takes a value and two quotes, it then applies each quote to
a copy of the value.

```
#100 [ n:inc ] [ n:dec ] bi
```

`tri` takes a value and three quotes. It then applies each quote
to a copy of the value.

```
#100 [ n:inc ] [ n:dec ] [ dup * ] tri
```

### Spread

Spread combinators apply multiple quotations to multiple values.
The asterisk suffixed to these function names signifies that
they are spread combinators.

`bi*` takes two values and two quotes. It applies the first
quote to the first value and the second quote to the second
value.

```
#1 #2 [ n:inc ] [ #2 * ] bi*
```

`tri*` takes three values and three quotes, applying the
first quote to the first value, the second quote to the
second value, and the third quote to the third value.

```
#1 #2 #3 [ n:inc ] [ #2 * ] [ n:dec ] tri*
```

### Apply

Apply combinators apply a single quotation to multiple values.
The @ sign suffixed to these function names signifies that they
are apply combinators.

`bi@` takes two values and a quote. It then applies the quote to
each value.

```
#1 #2 [ n:inc ] bi@
```

`tri@` takes three values and a quote. It then applies the quote
to each value.

```
#1 #2 #3 [ n:inc ] tri@
```

RETRO also provides `for-each` combinators for various data
structures. The exact usage of these varies; consult the
Glossary and relevant chapters for more details on these.

# Word Classes

Word classes are one of the two elements at the heart of
RETRO's interpreter.

There are different types of words in a Forth system. At a
minimum there are data words, regular words, and immediate
words. There are numerous approaches to dealing with this.

In RETRO I define special words which receive a pointer and
decide how to deal with it. These are grouped into a `class:`
namespace.

## How It Works

When a word is found in the dictionary, RETRO will push a
pointer to the definition (the `d:xt` field) to the stack
and then call the word specified by the `d:class` field.

The word called is responsible for processing the pointer
passed to it.

As a simple case, let's look at `immediate` words. These are
words which will always be called when encountered. A common
strategy is to have an immediacy bit which the interpreter
will look at, but RETRO uses a class for this. The class is
defined:

```
:class:immediate (a-)  call ;
```

Or a normal word. These should be called at interpret time
or compiled into definitions. The handler for this can look
like:

```
:class:word (a-) compiling? [ compile:call ] [ call ] choose ;
```

## Using Classes

The ability to add new classes is useful. If I wanted to add
a category of word that preserves an input value, I could do
it with a class:

```
:class:duplicating (a-)
  compiling? [ &dup compile:call ] [ &dup dip ] choose
  class:word ; 

:duplicating &class:duplicating reclass ;

:. n:put nl ; duplicating
#100 . . .
```

# Checking The Version

RETRO releases add and change things. You can use the `Version`
variable to determine the version in use and react accordingly.

```
@Version #201906 eq? [ 'Needs_2019.6! s:put nl bye ] if
```

This can be also be used to conditionally load compatibility files:

```
(If_newer_than_2016.6,_load_aliases_for_renamed_words)
@Version #201906 gt? [ 'Renamed_2019.6.forth include ] if
```

## Version Number Format

The version is a six digit number encoding the year and month of
the release. So:

    201901  is  2019.1
    201906  is  2019.6
    201911  is  2019.11

A `#100 /mod` will suffice to split these if needed.

# Errors

RETRO does only minimal error checking.

## Non-Fatal

A non-fatal error will be reported on *word not found* during
interactive or compile time. Note that this only applies to
calls: if you try to get a pointer to an undefined word, the
returned pointer will be zero.

## Fatal

A number of conditions are known to cause fatal errors. The
main ones are stack overflow, stack underflow, and division
by zero.

On these, RETRO will generally exit. For stack depth issues,
the VM will attempt to display an error prior to exiting.

In some cases, the VM may get stuck in an endless loop. If this
occurs, try using CTRL+C to kill the process, or kill it using
whatever means your host system provides.

## Rationale

Error checks are useful, but slow - especially on a minimal
system like RETRO. The overhead of doing depth or other checks
adds up quickly.

As an example, adding a depth check to `drop` increases the
time to use it 250,000 times in a loop from 0.16 seconds to
1.69 seconds.


# Lexical Scope

RETRO has a single dictionary, but does provide a means of using
lexical scope to keep this dictionary clean.

## Example

```
{{
  'A var
  :++A  &A v:inc ;
---reveal---
  :B ++A ++A @A n:put nl ;
}}
```

In this example, the lexical namespace is created with `{{`. A
variable (`A`) and word (`++A`) are defined. Then a marker is
set with `---reveal---`. Another word (`B`) is defined, and the
lexical area is closed with `}}`.

The headers between `{{` and `---reveal---` are then hidden from
the dictionary, leaving only the headers between `---reveal---`
and `}}` exposed.

## Notes

This only affects word visibility within the scoped area. As an
example:

```
:a #1 ;

{{
  :a #2 ;
---reveal---
  :b 'a s:evaluate n:put ;
}}
```

In this, after `}}` closes the area, the `:a #2 ;` is hidden and
the `s:evaluate` will find the `:a #1 ;` when `b` is run.

# The Stacks

The stacks are a defining feature of Forth. They are are used
to pass data between words and to track return addresses for
function calls.

RETRO always has two stacks, and optionally (if built with
floating point support) a third.

## Data Stack

This is the primary stack. Values are placed here, passed to
words which consume them and then return results. When I
refer to "the stack", this is the one I mean. Learning to use
the stack is a crucial part to making effective use of RETRO.

### Placing Values On The Stack

Values can be placed on the stack directly.

    | Example        | Action                                   |
    | -------------- | ---------------------------------------- |
    | `#300123`      | Push the number `300123` to the stack    |
    | `$h`           | Push the ASCII code for `h` to the stack |
    | `'hello_world` | Push a pointer to a string to the stack  |
    | `&fetch`       | Push the address of `fetch` to the stack |

### Reordering The Stack

RETRO provides a number of *shufflers* for reordering items
on the stack.

Some of the most common ones are:

    | Word    | Before   | After    |
    | ------- |--------- | -------- |
    | dup     | #1       | #1 #1    |
    | drop    | #1 #2    | #1       |
    | swap    | #1 #2    | #2 #1    |
    | over    | #1 #2    | #1 #2 #1 |
    | tuck    | #1 #2    | #2 #1 #2 |
    | nip     | #1 #2    | #2       |
    | rot     | #1 #2 #3 | #3 #1 #2 |

You can use `push` and `pop` to move values to and from the
address stack. Make sure you `pop` them back before the word
ends or RETRO will crash. These two words can not be used
at the interpreter.

There is also a special one, `reorder`, which allows for big
stack restructuring. This is slow but can be very useful.

As an example, let's say we have four values:

    #1 #2 #3 #4

And we want them to become:

    #4 #3 #2 #1

Doing this with the basic shufflers is difficult. You could end
up with something similar to:

    swap rot push rot pop swap 

But with `reorder`, you can just express the before and after
states:

    'abcd 'dcba reorder

### Resetting The Stack

If you need to quickly empty the stack, use `reset`.

### Get The Stack Depth

To find out how many items are on the stack, use `depth`.

### Displaying The Stack

You can display the stack by running `dump-stack`.

### Data Flow Combinators

RETRO provides *combinators* for working with data order on
the stack. These are covered in a later chapter and are worth
learning to use as they can help provide a cleaner, more
structured means of working.

### Tips

The stack is *not* an array in addressable memory. Don't try
to treat it like one.

## Address Stack

This stack primarily holds return addresses for function calls.
You normally won't need to directly interact with this stack,
but you can use `push` and `pop` to move values between the
data stack and this.

## Floating Point Stack

If you are using a build with floating point support a third
stack will be present. Floating point values are kept and
passed between words using this.

See the Floating Point chapter for more details on this.

## Tips

I recommend keeping the data stack shallow. Don't try to juggle
too much; it's better to factor definitions into shorter ones
that deal with simpler parts of the stack values than to have
a big definition with a lot of complex shuffling.

## Notes

The standard system is configured with a very deep data stack
(around 2,000 items) and an address stack that is 3x deeper.
In actual use, your programs are unlikely to ever need this,
but if you do, keep the limits in mind.

# Internals: Nga Virtual Machine

## Overview

At the heart of RETRO is a simple MISC (minimal instruction
set computer) processor for a dual stack architecture.

This is a very simple and straightforward system. There are
30 instructions. The memory is a linear array of signed 32
bit values. And there are two stacks: one for data and one
for return addresses.

## Instruction Table

                                         |     Stacks      |
    | Opcode | Muri | Full Name          | Data  | Address |
    | ------ | ---- | ------------------ | ----- | ------- |
    |  0     | ..   | nop                |   -   |   -     |
    |  1     | li   | lit                |   -n  |   -     |
    |  2     | du   | dup                |  n-nn |   -     |
    |  3     | dr   | drop               |  n-   |   -     |
    |  4     | sw   | swap               | xy-yx |   -     |
    |  5     | pu   | push               |  n-   |   -n    |
    |  6     | po   | pop                |   -n  |  n-     |
    |  7     | ju   | jump               |  a-   |   -     |
    |  8     | ca   | call               |  a-   |   -A    |
    |  9     | cc   | conditional call   | af-   |   -A    |
    | 10     | re   | return             |   -   |  A-     |
    | 11     | eq   | equality           | xy-f  |   -     |
    | 12     | ne   | inequality         | xy-f  |   -     |
    | 13     | lt   | less than          | xy-f  |   -     |
    | 14     | gt   | greater than       | xy-f  |   -     |
    | 15     | fe   | fetch              |  a-n  |   -     |
    | 16     | st   | store              | na-   |   -     |
    | 17     | ad   | addition           | xy-n  |   -     |
    | 18     | su   | subtraction        | xy-n  |   -     |
    | 19     | mu   | multiplication     | xy-n  |   -     |
    | 20     | di   | divide & remainder | xy-rq |   -     |
    | 21     | an   | bitwise and        | xy-n  |   -     |
    | 22     | or   | bitwise or         | xy-n  |   -     |
    | 23     | xo   | bitwise xor        | xy-n  |   -     |
    | 24     | sh   | shift              | xy-n  |   -     |
    | 25     | zr   | zero return        |  n-?  |   -     |
    | 26     | ha   | halt               |   -   |   -     |
    | 27     | ie   | i/o enumerate      |   -n  |   -     |
    | 28     | iq   | i/o query          |  n-xy |   -     |
    | 29     | ii   | i/o invoke         | ...n- |   -     |

## Encoding

Up to four instructions can be packed into each memory cell.

As an example,

    Opcode 4 Opcode 3 Opcode 2 Opcode 1
    00000000:00000000:00000000:00000000

If we have a bundle of `duliswst`, it would look like:

    st       sw       li       du
    00010000:00000100:00000001:00000010

Each `li` should have a value in the following cell(s). These
values will be pushed to the stack. E.g., `lili....` and
1, 2:

    00000000:00000000:00000001:00000001
    00000000 00000000 00000000 00000001 (1)
    00000000 00000000 00000000 00000010 (2)

## Shifts

`sh` performs a bitwise arithmetic shift operation.

This takes two values:

    xy

And returns a single one:

    z

If y is positive, this shifts `x` right by `y` bits. If negative,
it shifts left.

## Queries: Memory, Stacks

The `fe` instruction allows queries of some data related to
the Nga VM state. These are returned by reading from negative
addresses:

    | Address | Returns                |
    | ------- | ---------------------- |
    | -1      | Data stack depth       |
    | -2      | Address stack depth    |
    | -3      | Maximum Image Size     |
    | -4      | Minimum Integer Value  |
    | -5      | Maximum Integer Value  |

## I/O Devices

Nga provides three instructions for interacting with I/O devices.
These are:

    ie   i/o enumerate    returns the number of attached devices
    iq   i/o query        returns information about a device
    ii   i/o interact     invokes an interaction with a device

As an example, with an implementation providing an output source,
a block storage system, and keyboard:

    ie    will return `3` since there are three i/o devices
    0 iq  will return 0 0, since the first device is a screen (0)
          with a version of 0
    1 iq  will return 1 3, since the second device is a block
          storage (3), with a version of 1
    2 iq  will return 0 1, since the third device is a keyboard
          (1), with a version of 0

In this case, some interactions can be defined:

    : c:put
    i liiire..
    d 0
    
    : c:get
    i liiire..
    d 2

Setup the stack, push the device ID to the stack, and then use
`ii` to invoke the interaction.

A RETRO system requires one I/O device (a generic output for a
single character). This must be the first device, and must have
a device ID of 0.

All other devices are optional and can be specified in any order.

The currently supported and reserved device identifiers are:

    | ID   | Device Type      | Notes                      |
    | ---- | ---------------- | -------------------------- |
    | 0000 | Generic Output   | Always present as device 0 |
    | 0001 | Keyboard         |                            |
    | 0002 | Floating Point   |                            |
    | 0003 | Block Storage    | Raw, 1024 cell blocks      |
    | 0004 | Filesystem       | Unix-style Files           |
    | 0005 | Network: Gopher  | Make gopher requests       |
    | 0006 | Network: HTTP    | Make HTTP requests         |
    | 0007 | Network: Sockets |                            |
    | 0008 | Syscalls: Unix   |                            |
    | 0009 | Scripting Hooks  |                            |
    | 0010 | Random Number    |                            |

This list may be revised in the future. The only guaranteed
stable indentifier is 0000 for generic output.

# Internals: Interface Layers

Nga provides a virtual processor and an extensible way of adding
I/O devices, but does not provide any I/O itself. Adding I/O is
the responsability of the *interface layer*.

An interface layer will wrap Nga, providing at least one I/O
device (a generic output target), and a means of interacting
with the *retro image*.

It's expected that this layer will be host specific, adding any
system interactions that are needed via the I/O instructions.
The image will typically be extended with words to use these.



# Internals: I/O

RETRO provides three words for interacting with I/O. These are:

    io:enumerate    returns the number of attached devices
    io:query        returns information about a device
    io:invoke       invokes an interaction with a device

As an example, with an implementation providing an output source,
a block storage system, and keyboard:

    io:enumerate    will return `3` since there are three
                    i/o devices
    #0 io:query     will return 0 0, since the first device
                    is a screen (type 0) with a version of 0
    #1 io:query     will return 1 3, since the second device is
                    block storage (type 3), with a version of 1
    #2 io:query     will return 0 1, since the last device is a
                    keyboard (type 1), with a version of 0

In this case, some interactions can be defined:

    :c:put #0 io:invoke ;
    :c:get #2 io:invoke ;

Setup the stack, push the device ID, and then use `io:invoke`
to invoke the interaction.

A RETRO system requires one I/O device (a generic output for a
single character). This must be the first device, and must have
a device ID of 0.

All other devices are optional and can be specified in any
order.


# Internals: The Retro Image

The actual RETRO language is stored as a memory image for Nga.

## Format

The image file is a flat, linear sequence of signed 32-bit
values. Each value is stored in little endian format. The
size is not fixed. An interface should check when loading to
ensure that the physical image is not larger than the emulated
memory.

## Header

The image will start with two cells. The first is a liju....
instruction, the second is the target address for the jump.
This serves to skip over the rest of the data and reach the
actual entry point.

This is followed by a pointer to the most recent dictionary
header, a pointer to the next free address in memory, and
then the RETRO version number.

    | Offset | Contains                    |
    | ------ | --------------------------- |
    | 0      | lit call nop nop            |
    | 1      | Pointer to main entry point |
    | 2      | Dictionary                  |
    | 3      | Heap                        |
    | 4      | RETRO version               |

The actual code starts after this header.

The version number is the year and month. As an example,
the 12.2019.6 release will have a version number of
`201906`.

## Layout

Assuming an Nga built with 524287 cells of memory:

    | RANGE           | CONTAINS                     |
    | --------------- | ---------------------------- |
    | 0 - 1024        | RETRO Core kernel            |
    | 1025 - 1535     | token input buffer           |
    | 1536 +          | start of heap space          |
    | ............... | free memory for your use     |
    | 506879          | buffer for string evaluate   |
    | 507904          | temporary strings (32 * 512) |
    | 524287          | end of memory                |

The buffers at the end of memory will resize when specific
variables related to them are altered.

# Historical Papers and Notes

## On the Naming of RETRO

Taken from http://lists.tunes.org/archives/tunes-lll/1999-July/000121.html

On Fri, Jul 30, 1999 at 07:43:54PM -0400, Paul Dufresne wrote:

> My brother did found it funny that Retro is called like that.
> For him retro means going back (generally in time) so this
> does not looks like a name of a OS to come. So he'd like to
> know from where the name came.

Heheh, here's the story: When I started playing with OS stuff
last year (not seriously), I was reading about some old things
like FORTH and ITS, dating back to the 1960's and 70's.  The
past few years in America, there's been a revival of disco
music (along with bell bottoms, platform shoes, and all that
crap) and they call it "retro".  Now, my OS was named by
musicians.. I was telling a fellow musician about my ideas,
how it would be cool to have a small OS that isn't bloated and
unmanageable like Windows... go back to the 70's and resurrect
a line of software that died out.  He goes "hmm.. sounds kinda
retro.."

I think it sounds kinda rebellious, which is a Good Thing now
that everybody hates the M$ empire. :) It seems like other
people are as sick of the future as I am.  Look at TUNES, the
idea there isn't to make some great new invention, just take
some decades-old ideas and combine them in one OS.  The first
time I saw Knuth's "Art of Computer Programming" in the library
I thought "god that looks old.. 1973!!! nevermind.."  Now it's
my programming bible.  Find me something better published in
the 90's.. if such a thing exists, it'll be like a needle in a
haystack.  "Newer" doesn't necessarily mean "better".

	New cars = flimsier
	New farming methods = more devastating
	New version of Netscape = more bloat, more bullshit

One thing is better now: computer hardware.  Give me 70's
software on 90's and 00's hardware :)

- Tom Novelli <tcn@tunes.org>


## The Design Philosophy of RETRO Native Forth

Computer software is a technology in its infancy, a mere fifty years
old.  The last 25 years in particular have seen an explosion in the
software business.  However, software has seen little innovation while
hardware technology has improved phenomenally (notwithstanding the advent
of lousy slave-made parts).  Proven software techniques of forty years ago
have yet to reach widespread use, in deference to the "latest and
greatest"  proprietary solutions of dubious value.  Thanks to agressive
marketing, we make huge investments in these dead-end technologies
(through our businesses and governments, if not personally) and we end up
with a reliance on a heap of complicated, error-prone, poorly understood
junk software.

Complexity will dominate the software industry for the foreseeable
future.  The Retro philosophy is a simple alternative for those willing to
make a clean break with legacy software.  A Retro system can communicate
with other systems, but it won't run much legacy software, especially
proprietary software without source code.  An emulation layer could be
added, but doing so would defeat the purpose of a simple operating system.
I think TCP/IP support is all the compatibility that's needed.

At first Retro will appeal to computer hobbyists and electronic
engineers.  Once the rough edges are smoothed out, it could catch on with
ordinary folks who don't like waiting five minutes just to check their
email (not to mention the long hours of setup and maintenance).  Game
programmers who take their craft seriously may also be interested.
Businesses might even see a use for it, if the managers decide it's more
cost-effective to carefully design software for specific needs, rather
than buying off-the-shelf crap and spending countless manhours working
around the bugs.  Since it's not practical for businesses to make a clean
break, my advice is to run Retro (and its ilk) on separate machines
connected by a network.  Retro is efficient enough to run on older
machines that would otherwise sit idle, being too slow for the latest
Microsoft bloatware (or Linux, for that matter).

I strive to avoid the extraneous.  That applies even to proven
technologies, if I don't need them.  If my computer isn't set up for
people to log in over the network, I don't want security features; they
just get in the way.  If I'm only running programs I wrote, I should be
able to run them with full access to the hardware; I don't need protection
from viruses.  If I download something I don't trust, then I can run it in
an isolated process, which is customary with Unix and kin.  But that's not
core functionality.  All that's needed is the flexibility to add things
like security, graphical interfaces, and distributed processing - if the
need ever arises.

In programming languagues, I was misled.  It's the Tower of Babel all
over again.  The thousands of languages in existence all fall into a
handful of archetypes: Assembler, LISP, FORTRAN and FORTH represent the
earliest descendants of nearly all languages.  I hesitate to name a
definitive "object-oriented" language, and here's why: Object-Oriented
programming is just a technique, and any language will suffice, even
Assembler.  The complexites of fancy languages like Ada and C++ are a
departure from reality -- the reality of the actual physical machine.
When it all boils down, even LISP, FORTRAN and FORTH are only extensions
of the machine.

I chose FORTH as the "native tongue" of Retro.  LISP, FORTRAN, and
other languages can be efficiently implemented as extensions of FORTH, but
the reverse isn't so efficient.  Theoretically all languages are
equivalent, but when design time, compilation time, and complexity are
accounted for, FORTH is most efficient.  FORTH also translates most
directly to the hardware.  (In fact, FORTH has been implemented in
hardware; these "stack machines" are extremely efficient.)  FORTH is also
the easiest language to implement from scratch - a major concern when
you're trying to make a clean break.  So with simplicity in mind, FORTH
was the obvious choice.

I'm perfectly happy working with text only, and I go to great lengths
to avoid using the standard graphical environments, which have major
problems: windows, pulldown menus, and mice.  Windows can't share the
screen nicely; that idea is hopeless.  Pulldowns are tedious.  Mice get in
the way of typing without reducing the need for it; all they give me is
tendonitis.  Their main use is for drawing.

Some of my favorite interfaces: Telix, Telegard BBS, Pine, Pico, Lynx,
and ScreamTracker.  All "hotkey" interfaces where you press a key or two
to perform an action.  Usually the important commands are listed at the
bottom of the screen, or at least on a help screen.  The same principles
apply to graphical interfaces: use the full screen, except for a status
and menu area on one edge.  Resist the temptation to clutter up the
screen.

As for switching between programs, the Windows methods suck; the only
thing worse is Unix job control (jobs, fg, and such).  The Linux method is
tolerable: Alt-Arrows, Alt-F1, Alt-F2, etc.  Still, things could be
better: F11 and F12 cycle back and forth through all open programs; Alt-F1
assigns the currently selected program to F1, and likewise for the other
function keys.  Programs just won't use function keys - Control and Alt
combinations are less awkward and easier to remember, besides.  I'll also
want a "last channel" key and a "task list" key; maybe I'll borrow those
stupid Win95 keys.  The Pause key will do like it says - pause the current
program - and Ctrl-Pause (Break) will kill it.

One more thing: consistency.  I like programs to look different so I
can tell them apart, but the keys should be the same as much as possible.
Keys should be configured in one place, for all programs.  Finally,
remember the most consistent interface, one of the few constants
throughout the history of computing - the text screen and keyboard, and
the teletypewriter before that.  Don't overlook it.

More to come, maybe... :)

"If it's on line, it's a work in progress."

Tom Novelli, 3/4/2000

## Metacompilation and Assembly

RETRO 10 and 11 were written in themselves using a metacompiler.
I had been fascinated by this idea for a long time and was able
to explore it heavily. While I still find it to be a good idea,
the way I ended up doing it was problematic.

The biggest issue I faced was that I wanted to do this in one
step, where loading the RETRO source would create a new image
in place of the old one, switch to the new one, and then load
the higher level parts of the language over this. In retrospect,
this was a really bad idea.

My earlier design for RETRO was very flexible. I allowed almost
everything to be swapped out or extended at any time. This made
it extremely easy to customize the language and environment, but
made it crucial to keep track of what was in memory and what had
been patched so that the metacompiler wouldn't refer to anything
in the old image during the relocation and control change. It
was far too easy to make a mistake, discover that elements of
the new image were broken, and then have to go and revert many
changes to try to figure out what went wrong.

This was also complicated by the fact that I built new images
as I worked, and, while a new image could be built from the last
built one, it wasn't always possible to build a new image from
the prior release version. (Actually, it was often worse - I
failed to check in every change as I went, so often even the
prior commits couldn't rebuild the latest images).

For RETRO 12 I wanted to avoid this problem, so I decided to go
back to writing the kernel ("Rx") in assembly. I actually wrote
a Machine Forth dialect to generate the initial assembly, before
eventually hand tuning the final results to its current state.

I could (and likely will eventually) write the assembler in
RETRO, but the current one is in C, and is built as part of the
standard toolchain.

My VM actually has two assemblers. The older one is Naje. This
was intended to be fairly friendly to work with, and handles
many of the details of packing instructions for the user. Here
is an example of a small program in it:

    :square
      dup
      mul
      ret
    :main
      lit 35
      lit &square
      call
      end

The other assembler is Muri. This is a far more minimalistic
assembler, but I've actually grown to prefer it. The above
example in Muri would become:

    i liju....
    r main
    : square
    i dumure..
    : main
    i lilica..
    d 35
    r square
    i en......

In Muri, each instruction is reduced to two characters, and the
bundlings are listed as part of an instruction bundle (lines
starting with `i`). This is less readable if you aren't very
familiar with Nga's assembly and packing rules, but allows a
very quick, efficient way of writing assembly for those who are.

I eventually rewrote the kernel in the Muri style as it's what
I prefer, and since there's not much need to make changes in it.

## The Path to Self Hosting

RETRO is an image based Forth system running on a lightweight
virtual machine. This is the story of how that image is made.

The first RETRO to use an image based approach was RETRO 10.
The earliest images were built using a compiler written in
Toka, an earlier experimental stack language I had written.
It didn't take long to want to drop the dependency on Toka,
so I rewrote the image compiler in RETRO and then began
development at a faster pace.

RETRO 11 was built using the last RETRO 10 image and an
evolved version of the metacompiler. This worked well, but
I eventually found it to be problematic.

One of the issues I faced was the inability to make a new
image from the prior stable release. Since I develop and
test changes incrementally, I reached a point where the
current metacompiler and image required each other. This
wasn't a fatal flaw, but it was annoying.

Perhaps more critical was the fragility of the system. In
R11 small mistakes could result in a corrupt image. The test
suite helped identify some of these, but there were a few
times I was forced to dig back through the version control
history to recover a working image.

The fragile nature was amplified by some design decisions.
In R11, after the initial kernel was built, it would be
moved to memory address 0, then control would jump into the
new kernel to finish building the higher level parts.

Handling this was a tricky task. In R11 almost everything
could be revectored, so the metacompiler had to ensure that
it didn't rely on anything in the old image during the move.
This caused a large number of issues over R11's life.

So on to RETRO 12. I decided that this would be different.
First, the kernel would be assembly, with an external tool
to generate the core image. The kernel is in `Rx.md` and the
assembler is `Muri`. To load the standard library, I wrote a
second tool, `retro-extend`. This separation has allowed me
many fewer headaches as I can make changes more easily and
rebuild from scratch when necessary.

But I miss self-hosting. So last fall I decided to resolve
this. And today I'm pleased to say that it is now done.

There are a few parts to this.

**Unu**. I use a Markdown variation with fenced code blocks.
The tool I wrote in C to extract these is called `unu`. For
a self hosting RETRO, I rewrote this as a combinator that
reads in a file and runs another word against each line in the
file. So I could display the code block contents by doing:

    'filename [ s:put nl ] unu

This made it easier to implement the other tools.

**Muri**. This is my assembler. It's minimalistic, fast, and
works really well for my purposes. RETRO includes a runtime
version of this (using `as{`, `}as`, `i`, `d`, and `r`), so
all I needed for this was to write a few words to parse the
lines and run the corresponding runtime words. As with the C
version, this is a two pass assembler.

Muri generates a new `ngaImage` with the kernel. To create a
full image I needed a way to load in the standard library and
I/O extensions.

This is handled by **retro-extend**. This is where it gets
more complex. I implemented the Nga virtual machine in RETRO
to allow this to run the new image in isolation from the
host image. The new ngaImage is loaded, the interpreter is
located, and each token is passed to the interpreter. Once
done, the new image is written to disk.

So at this point I'm pleased to say that I can now develop
RETRO using only an existing copy of RETRO (VM+image) and
tools (unu, muri, retro-extend, and a line oriented text
editor) written in RETRO.

This project has delivered some additional side benefits.
During the testing I was able to use it to identify a few
bugs in the I/O extensions, and the Nga-in-RETRO will replace
the older attempt at this in the debugger, allowing a safer
testing environment.

What issues remain?

The extend process is *slow*. On my main development server
(Linode 1024, OpenBSD 6.4, 64-bit) it takes a bit over five
minutes to complete loading the standard library, and a few
additional depending on the I/O drivers selected.

Most of the performance issues come from running Nga-in-RETRO
to isolate the new image from the host one. It'd be possible
to do something a bit more clever (e.g., running a RETRO
instance using the new image via a subprocess and piping in
the source, or doing relocations of the data), but this is
less error prone and will work on all systems that I plan to
support (including, with a few minor adjustments, the native
hardware versions [assuming the existance of mass storage]).

Sources:

**Unu**

- http://forth.works/c8820f85e0c52d32c7f9f64c28f435c0
- gopher://forth.works/0/c8820f85e0c52d32c7f9f64c28f435c0

**Muri**

- http://forth.works/09d6c4f3f8ab484a31107dca780058e3
- gopher://forth.works/0/09d6c4f3f8ab484a31107dca780058e3

**retro-extend**

- http://forth.works/c812416f397af11db58e97388a3238f2
- gopher://forth.works/0/c812416f397af11db58e97388a3238f2

## Prefixes as a Language Element

A big change in RETRO 12 was the elimination of the traditional
parser from the language. This was a sacrifice due to the lack
of an I/O model. RETRO has no way to know *how* input is given
to the `interpret` word, or whether anything else will ever be
passed into it.

And so `interpret` operates only on the current token. The core
language does not track what came before or attempt to guess at
what might come in the future.

This leads into the prefixes. RETRO 11 had a complicated system
for prefixes, with different types of prefixes for words that
parsed ahead (e.g., strings) and words that operated on the
current token (e.g., `@`). RETRO 12 eliminates all of these in
favor of just having a single prefix model.

The first thing `interpret` does is look to see if the first
character in a token matches a `prefix:` word. If it does, it
passes the rest of the token as a string pointer to the prefix
specific handler to deal with. If there is no valid prefix
found, it tries to find it in the dictionary. Assuming that it
finds the words, it passes the `d:xt` field to the handler that
`d:class` points to. Otherwise it calls `err:notfound`.

This has an important implication: *words can not reliably
have names that start with a prefix character.*

It also simplifies things. Anything that would normally parse
becomes a prefix handler. So creating a new word? Use the `:`
prefix. Strings? Use `'`. Pointers? Try `&`. And so on. E.g.,

    In ANS                  | In RETRO
    : foo ... ;             | :foo ... ;
    ' foo                   | &foo
    : bar ... ['] foo ;     | :bar ... &foo ;
    s" hello world!"        | 'hello_world!

If you are familiar with ColorForth, prefixes are a similar
idea to colors, but can be defined by the user as normal words.

After doing this for quite a while I rather like it. I can see
why Chuck Moore eventually went towards ColorForth as using
color (or prefixes in my case) does simplify the implementation
in many ways.

## On The Kernel Wordset

In implementing the RETRO 12 kernel (called RETRO Core, and
defined in `image/retro.muri`) I had to decide on what functionality
would be needed. It was important to me that this be kept clean
and minimalistic, as I didn't want to spend a lot of time
changing it as time progressed. It's far nicer to code at the
higher level, where the RETRO language is fully functional, as
opposed to writing more assembly code.

So what made it in?

Primitives

These are words that map directly to Nga instructions.

    dup      drop     swap   call   eq?   -eq?   lt?   gt?
    fetch    store    +      -      *     /mod   and   or
    xor      shift    push   pop    0;

Memory

    fetch-next    store-next    ,    s,

Strings

    s:to-number    s:eq?    s:length

Flow Control

    choose    if    -if    repeat    again

Compiler & Interpreter

    Compiler    Heap        ;       [    ]      Dictionary
    d:link      d:class     d:xt    d:name      d:add-header
    class:word  class:primitive     class:data  class:macro
    prefix::    prefix:#    prefix:&    prefix:$
    interpret   d:lookup    err:notfound

Assembler

    i           d           r

I *could* slightly reduce this. The $ prefix could be defined in
higher level code, and I don't strictly *need* to expose the
`fetch-next` and `store-next` here. But since the are already
implemented as dependencies of the words in the kernel, it would
be a bit wasteful to redefine them later in higher level code.

A recent change was the addition of the assembler into the
kernel. This allows the higher levels to use assembly as needed,
which gives more flexibility and allows for more optimal code
in the standard library.

With these words the rest of the language can be built up. Note
that the RETRO kernel does not provide any I/O words. It's assumed
that the RETRO interfaces will add these as best suited for the
systems they run on.

There is another small bit. All images start with a few key
pointers in fixed offsets of memory. These are:

    | Offset | Contains                    |
    | ------ | --------------------------- |
    | 0      | lit call nop nop            |
    | 1      | Pointer to main entry point |
    | 2      | Dictionary                  |
    | 3      | Heap                        |
    | 4      | RETRO version identifier    |

An interface can use the dictionary pointer and knowledge of the
dictionary format for a specific RETRO version to identify the
location of essential words like `interpret` and `err:notfound`
when implementing the user facing interface.

## On The Evolution Of Ngaro Into Nga

When I decided to begin work on what became RETRO 12, I knew
the process would involve updating Ngaro, the virtual machine
that RETRO 10 and 11 ran on.

Ngaro rose out of an earlier experimental virtual machine I had
written back in 2005-2006. This earlier VM, called Maunga, was
very close to what Ngaro ended up being, though it had a very
different approach to I/O. (All I/O in Maunga was intended to be
memory mapped; Ngaro adopted a port based I/O system).

Ngaro itself evolved along with RETRO, gaining features like
automated skipping of NOPs and a LOOP opcode to help improve
performance. But the I/O model proved to be a problem. When I
created Ngaro, I had the idea that I would always be able to
assume a console/terminal style environment. The assumption was
that all code would be entered via the keyboard (or maybe a
block editor), and that proved to be the fundamental flaw as
time went on.

As RETRO grew it was evident that the model had some serious
problems. Need to load code from a file? The VM and language had
functionality to pretend it was being typed in. Want to run on
something like a browser, Android, or iOS? The VM would need to
be implemented in a way that simulates input being typed into
the VM via a simulated keyboard. And RETRO was built around this.
I couldn't change it because of a promise to maintain, as much
as possible, source compatibility for a period of at least five
years.

When the time came to fix this, I decided at the start to keep
the I/O model separate from the core VM. I also decided that the
core RETRO language would provide some means of interpreting
code without requiring an assumption that a traditional terminal
was being used.

So Nga began. I took the opportunity to simplify the instruction
set to just 26 essential instructions, add support for packing
multiple instructions per memory location (allowing a long due
reduction in memory footprint), and to generally just make a far
simpler design.

I've been pleased with Nga. On its own it really isn't useful
though. So with RETRO I embed it into a larger framework that
adds some basic I/O functionality. The *interfaces* handle the
details of passing tokens into the language and capturing any
output. They are free to do this in whatever model makes most
sense on a given platform.

So far I've implemented:

    - a scripting interface, reading input from a file and
      offering file i/o, gopher, and reading from stdin, and
      sending output to stdout.
    - an interactive interface, built around ncurses, reading
      input from stdin, and displaying output to a scrolling
      buffer.
    - an iOS interface, built around a text editor, directing
      output to a separate interface pane.
    - an interactive block editor, using a gopher-based block
      data store. Output is displayed to stdout, and input is
      done via the blocks being evaluated or by reading from
      stdin.

In all cases, the only common I/O word that has to map to an
exposed instruction is `putc`, to display a single character to
some output device. There is no requirement for a traditional
keyboard input model.

By doing this I was able to solve the biggest portability issue
with the RETRO 10/11 model, and make a much simpler, cleaner
language in the end.

## RETRO 11 (2011 - 2019): A Look Back

So it's now been about five years since the last release of RETRO
11. While I still see some people obtaining and using it, I've
moved on to the twelth generation of RETRO. It's time for me to
finally retire RETRO 11.

As I prepare to do so, I thought I'd take a brief look back.

RETRO 11 began life in 2011. It grew out of RETRO 10, which was
the first version of RETRO to not be written in x86 assembly
language. For R10 and R11, I wrote a portable virtual machine
(with numerous implementations) and the Forth dialect was kept
in an image file which ran on the VM.

RETRO 10 worked, but was always a bit too sloppy and changed
drastically between releases. The major goal of RETRO 11 was to
provide a stable base for a five year period. In retrospect,
this was mostly achieved. Code from earlier releases normally
needed only minor adjustments to run on later releases, though
newer releases added significantly to the language.

There were seven releases.

- Release 11.0: 2011, July
- Release 11.1: 2011, November
- Release 11.2: 2012, January
- Release 11.3: 2012, March
- Release 11.4: 2012, July
- Release 11.5: 2013, March
- Release 11.6: 2014, August

Development was fast until 11.4. This was the point at which I
had to slow down due to RSI problems. It was also the point
which I started experiencing some problems with the metacompiler
(as discussed previously).

RETRO 11 was flexible. All colon definitions were setup as hooks,
allowing new functionality to be layered in easily. This allowed
the later releases to add things like vocabularies, search order,
tab completion, and keyboard remapping. This all came at a cost
though: later things could use the hooks to alter behavior of
existing words, so it was necessary to use a lot of caution to
ensure that the layers didn't break the earlier code.

The biggest issue was the I/O model. RETRO 11 and the Ngaro VM
assumed the existence of a console environment. All input was
required to be input at the keyboard, and all output was to be
shown on screen. This caused some problems. Including code from
a file required some tricks, temporarily rewriting the keyboard
input function to read from the file. It also became a major
issue when I wrote the iOS version. The need to simulate the
keyboard and console complicated everything and I had to spend
a considerable amount of effort to deal with battery performance
resulting from the I/O polling and wait states.

But on the whole it worked well. I used RETRO 11.6 until I started
work on RETRO 12 in late 2016, and continued running some tools
written in R11 until the first quarter of last year.

The final image file was 23,137 cells (92,548 bytes). This was
bloated by keeping some documentation (stack comments and short
descriptions) in the image, which started in 11.4. This contained
269 words.

I used RETRO 11 for a wide variety of tasks. A small selection of
things that were written includes:

- a pastebin
- front end to ii (irc client)
- small explorations of interactive fiction
- irc log viewer
- tool to create html from templates
- tool to automate creation of an SVCD from a set of photos
- tools to generate reports from data sets for my employer

In the end, I'm happy with how RETRO 11 turned out. I made some
mistakes in embracing too much complexity, but despite this it
was a successful system for many years.

# Security Concerns

The standard RETRO is not a good choice for applications
needing to be highly secure.

## Runtime Checks

The RETRO system performs only minimal checks. It will not
load an image larger than the max set at build time. And
stack over/underflow are checked for as code executes.

The system does not attempt to validate anything else, it's
quite easy to crash.

## Isolation

The VM itself and the core code is self contained. Nga does
not make use of malloc/free, and uses only standard system
libraries. It's possible for buffer overruns within the image
(overwriting Nga code), but the RETRO image shouldn't leak
into the C portions.

I/O presents a bigger issue. Anything involving I/O, especially
with the `unix:` words, may be a vector for attacks.

## Future Direction

I'm not planning to add anything to the *image* side as, for me,
the performance hit due to added checks is bigger than the
benefits.

The story is different on the VM side. I've already begun taking
steps to address some of the issues, using functions that check
for overruns with strings and doing some minor testing for these
conditions. I will be gradually addressing the various I/O
related extensions, though it's unlikely to ever be fully guarded
against attacks.

## Rationale

RETRO is, primarily, a personal system. I'm running code I wrote
to solve problems I face. On the occasions where I run code sent
to me by others, I read it carefully first and then run inside a
sandboxed environment if I'm worried about anything in it.

### On The Use Of Underscores In Word Names

In brief: don't use underscores in word names.

There is a good reason for this, and it has to do with how RETRO
processes strings. By default, underscores in strings are replaced
by spaces. This is problematic when dealing with words like `var`,
`const`, and `d:create` which take word names as strings.

Consider:

    :hello_msg 'hello_user ;
    'test_name var
    #188 !test_name

In the first case, the `:` prefix handles the token, so the
underscore is not remapped to a space, creating a word name as
`hello_msg`. But in the second, the `'` prefix remaps the
underscore to a space, giving a variable name of `test name`.
In the third line, the name lookup will fail as `test_name` is
not defined, so the store will be done to an incorrect address.

Because of this, it's best to avoid underscores in names.

Having covered this, if you do need to use them for some reason,
you can replace `d:add-header` with a version that remaps spaces
back to underscores before creating the header. The following
will allow for this.

    ~~~
    {{
      :fields        @Dictionary , (link) , (xt) , (class) ;
      :invalid-name? dup ASCII:SPACE s:contains-char? ;
      :rewrite       [ ASCII:SPACE [ $_ ] case ] s:map ;
      :entry         here &call dip !Dictionary ;
      [ [ fields invalid-name? &rewrite if s, (name) ] entry ]
    }}

    #1793 &d:add-header store
    &d:add-header n:inc store
    ~~~

Additional Note:

Some version of RETRO have included the above patch. The last
release that will include this by default is 2020.4 as it is
not needed by the majority of users. If you want to keep it in
your system, you will need to load it yourself or add it to
your `package/list.forth` (for Unix users) before building.

# The Code It Yourself Manifesto

We use software for our everyday needs because we want to get
something done. We have goals to achieve and things to do.

The software we use is coded by brave programmers that have
their own goals. Most of the time there is an overlap between
their goals and ours.

Over time these will diverge.

This means that the tools we depend on grow features we don't
use or understand. There will be bugs in these code parts which
will prevent us from reaching our goals.

So we are at a fork in the road:

- We have the choice of trying to understand the code and
  fix it.
- We have the choice of trying another program, whose
  creator's goals are closer to ours.
- We also have the choice of coding the software ourself.

All but the last path mean endless seeking, evaluating and
further deviation from our goals. Therefore we replace programs
we do not understand fully with our own implementation.

The followers of the Code It Yourself Manifesto believe in
these things:

- We implement it according to our own goals.
- We make mistakes and learn from them.
- We learn how our tools we depend on need to work.
- We gain a deep understanding of our problem domain.
- We still embrace sharing of ideas and code.

Sharing is only possible if we are excellent developers to
each other. The next developer reading our code will be us
in a not so distant future. Coding It Ourselves means we will
document our code, clearly stating the goal of the software
we write.

Together we enjoy the diversity of implementations and ideas.

We encourage our colleagues to

**Code It Yourself.**

----

Written by Christian Kellermann on 2016-01-12, licensed under
a CreativeCommonsAttribution-ShareAlike3.0UnportedLicense.

Original text taken from
http://pestilenz.org/~ckeen/blog/posts/ciy-manifesto.html

# Deprecation Policy

As RETRO evolves, some words will become obsolete and no longer
be needed. In each release, these will be marked as deprecated
in the glossary. Any deprecated words will be removed in the
next quarterly release.

E.g., if 2020.1 had deprecated words, these would be removed in
the 2020.4 release. Any words made deprecated in between 2020.1
and 2020.4 would be removed in the 2020.7 release.

The text in these files is Copyright (c) 2018-2020 by
Charles Childers.

To the extent possible under law, Charles Childers has
waived all copyright and related or neighboring rights
to the RETRO Documentation. This work is published from:
United States.

The historical papers are Copyright (c) 1999-2000 by
Tom Novelli.

## Legal Text

See https://creativecommons.org/publicdomain/zero/1.0/legalcode

The laws of most jurisdictions throughout the world automatically confer 
exclusive Copyright and Related Rights (defined below) upon the creator 
and subsequent owner(s) (each and all, an "owner") of an original work 
of authorship and/or a database (each, a "Work").

Certain owners wish to permanently relinquish those rights to a Work for 
the purpose of contributing to a commons of creative, cultural and 
scientific works ("Commons") that the public can reliably and without 
fear of later claims of infringement build upon, modify, incorporate in 
other works, reuse and redistribute as freely as possible in any form 
whatsoever and for any purposes, including without limitation commercial 
purposes. These owners may contribute to the Commons to promote the 
ideal of a free culture and the further production of creative, cultural 
and scientific works, or to gain reputation or greater distribution for 
their Work in part through the use and efforts of others.

For these and/or other purposes and motivations, and without any 
expectation of additional consideration or compensation, the person 
associating CC0 with a Work (the "Affirmer"), to the extent that he or 
she is an owner of Copyright and Related Rights in the Work, voluntarily 
elects to apply CC0 to the Work and publicly distribute the Work under 
its terms, with knowledge of his or her Copyright and Related Rights in 
the Work and the meaning and intended legal effect of CC0 on those 
rights.

1. Copyright and Related Rights. A Work made available under CC0 may be 
protected by copyright and related or neighboring rights ("Copyright and 
Related Rights"). Copyright and Related Rights include, but are not 
limited to, the following:

- the right to reproduce, adapt, distribute, perform, display, 
   communicate, and translate a Work;

- moral rights retained by the original author(s) and/or performer(s);

- publicity and privacy rights pertaining to a person's image or 
  likeness depicted in a Work;

- rights protecting against unfair competition in regards to a Work, 
  subject to the limitations in paragraph 4(a), below;

- rights protecting the extraction, dissemination, use and reuse of data 
  in a Work;

- database rights (such as those arising under Directive 96/9/EC of the 
  European Parliament and of the Council of 11 March 1996 on the legal 
  protection of databases, and under any national implementation thereof, 
  including any amended or successor version of such directive); and

- other similar, equivalent or corresponding rights throughout the world
  based on applicable law or treaty, and any national implementations 
  thereof.

2. Waiver. To the greatest extent permitted by, but not in contravention 
of, applicable law, Affirmer hereby overtly, fully, permanently, 
irrevocably and unconditionally waives, abandons, and surrenders all of 
Affirmer's Copyright and Related Rights and associated claims and causes 
of action, whether now known or unknown (including existing as well as 
future claims and causes of action), in the Work (i) in all territories 
worldwide, (ii) for the maximum duration provided by applicable law or 
treaty (including future time extensions), (iii) in any current or 
future medium and for any number of copies, and (iv) for any purpose 
whatsoever, including without limitation commercial, advertising or 
promotional purposes (the "Waiver"). Affirmer makes the Waiver for the 
benefit of each member of the public at large and to the detriment of 
Affirmer's heirs and successors, fully intending that such Waiver shall 
not be subject to revocation, rescission, cancellation, termination, or 
any other legal or equitable action to disrupt the quiet enjoyment of 
the Work by the public as contemplated by Affirmer's express Statement 
of Purpose.

3. Public License Fallback. Should any part of the Waiver for any reason 
be judged legally invalid or ineffective under applicable law, then the 
Waiver shall be preserved to the maximum extent permitted taking into 
account Affirmer's express Statement of Purpose. In addition, to the 
extent the Waiver is so judged Affirmer hereby grants to each affected 
person a royalty-free, non transferable, non sublicensable, non 
exclusive, irrevocable and unconditional license to exercise Affirmer's 
Copyright and Related Rights in the Work (i) in all territories 
worldwide, (ii) for the maximum duration provided by applicable law or 
treaty (including future time extensions), (iii) in any current or 
future medium and for any number of copies, and (iv) for any purpose 
whatsoever, including without limitation commercial, advertising or 
promotional purposes (the "License"). The License shall be deemed 
effective as of the date CC0 was applied by Affirmer to the Work. Should 
any part of the License for any reason be judged legally invalid or 
ineffective under applicable law, such partial invalidity or 
ineffectiveness shall not invalidate the remainder of the License, and 
in such case Affirmer hereby affirms that he or she will not (i) 
exercise any of his or her remaining Copyright and Related Rights in the 
Work or (ii) assert any associated claims and causes of action with 
respect to the Work, in either case contrary to Affirmer's express 
Statement of Purpose.

4. Limitations and Disclaimers.

No trademark or patent rights held by Affirmer are waived, abandoned, 
surrendered, licensed or otherwise affected by this document.

Affirmer offers the Work as-is and makes no representations or 
warranties of any kind concerning the Work, express, implied, statutory 
or otherwise, including without limitation warranties of title, 
merchantability, fitness for a particular purpose, non infringement, or 
the absence of latent or other defects, accuracy, or the present or 
absence of errors, whether or not discoverable, all to the greatest 
extent permissible under applicable law.

Affirmer disclaims responsibility for clearing rights of other persons 
that may apply to the Work or any use thereof, including without 
limitation any person's Copyright and Related Rights in the Work. 
Further, Affirmer disclaims responsibility for obtaining any necessary 
consents, permissions or other rights required for any use of the Work.

Affirmer understands and acknowledges that Creative Commons is not a 
party to this document and has no duty or obligation with respect to 
this CC0 or use of the Work.

----

The Code It Yourself Manifesto is Copyright (c) 2016 by
Christian Kellermann and is used under the 
Attribution-ShareAlike 3.0 Unported (CC BY-SA 3.0) license.

This license reads:

THE WORK (AS DEFINED BELOW) IS PROVIDED UNDER THE TERMS OF THIS CREATIVE 
COMMONS PUBLIC LICENSE ("CCPL" OR "LICENSE"). THE WORK IS PROTECTED BY 
COPYRIGHT AND/OR OTHER APPLICABLE LAW. ANY USE OF THE WORK OTHER THAN AS 
AUTHORIZED UNDER THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.

BY EXERCISING ANY RIGHTS TO THE WORK PROVIDED HERE, YOU ACCEPT AND AGREE 
TO BE BOUND BY THE TERMS OF THIS LICENSE. TO THE EXTENT THIS LICENSE MAY 
BE CONSIDERED TO BE A CONTRACT, THE LICENSOR GRANTS YOU THE RIGHTS 
CONTAINED HERE IN CONSIDERATION OF YOUR ACCEPTANCE OF SUCH TERMS AND 
CONDITIONS.

1. Definitions

"Adaptation" means a work based upon the Work, or upon the Work and 
other pre-existing works, such as a translation, adaptation, derivative 
work, arrangement of music or other alterations of a literary or 
artistic work, or phonogram or performance and includes cinematographic 
adaptations or any other form in which the Work may be recast, 
transformed, or adapted including in any form recognizably derived from 
the original, except that a work that constitutes a Collection will not 
be considered an Adaptation for the purpose of this License. For the 
avoidance of doubt, where the Work is a musical work, performance or 
phonogram, the synchronization of the Work in timed-relation with a 
moving image ("synching") will be considered an Adaptation for the 
purpose of this License.

"Collection" means a collection of literary or artistic works, such as 
encyclopedias and anthologies, or performances, phonograms or 
broadcasts, or other works or subject matter other than works listed in 
Section 1(f) below, which, by reason of the selection and arrangement of 
their contents, constitute intellectual creations, in which the Work is 
included in its entirety in unmodified form along with one or more other 
contributions, each constituting separate and independent works in 
themselves, which together are assembled into a collective whole. A work 
that constitutes a Collection will not be considered an Adaptation (as 
defined below) for the purposes of this License.

"Creative Commons Compatible License" means a license that is listed at 
https://creativecommons.org/compatiblelicenses that has been approved by 
Creative Commons as being essentially equivalent to this License, 
including, at a minimum, because that license: (i) contains terms that 
have the same purpose, meaning and effect as the License Elements of 
this License; and, (ii) explicitly permits the relicensing of 
adaptations of works made available under that license under this 
License or a Creative Commons jurisdiction license with the same License 
Elements as this License.

"Distribute" means to make available to the public the original and 
copies of the Work or Adaptation, as appropriate, through sale or other 
transfer of ownership.

"License Elements" means the following high-level license attributes as 
selected by Licensor and indicated in the title of this License: 
Attribution, ShareAlike.

"Licensor" means the individual, individuals, entity or entities that 
offer(s) the Work under the terms of this License.

"Original Author" means, in the case of a literary or artistic work, the 
individual, individuals, entity or entities who created the Work or if 
no individual or entity can be identified, the publisher; and in 
addition (i) in the case of a performance the actors, singers, 
musicians, dancers, and other persons who act, sing, deliver, declaim, 
play in, interpret or otherwise perform literary or artistic works or 
expressions of folklore; (ii) in the case of a phonogram the producer 
being the person or legal entity who first fixes the sounds of a 
performance or other sounds; and, (iii) in the case of broadcasts, the 
organization that transmits the broadcast.

"Work" means the literary and/or artistic work offered under the terms 
of this License including without limitation any production in the 
literary, scientific and artistic domain, whatever may be the mode or 
form of its expression including digital form, such as a book, pamphlet 
and other writing; a lecture, address, sermon or other work of the same 
nature; a dramatic or dramatico-musical work; a choreographic work or 
entertainment in dumb show; a musical composition with or without words; 
a cinematographic work to which are assimilated works expressed by a 
process analogous to cinematography; a work of drawing, painting, 
architecture, sculpture, engraving or lithography; a photographic work 
to which are assimilated works expressed by a process analogous to 
photography; a work of applied art; an illustration, map, plan, sketch 
or three-dimensional work relative to geography, topography, 
architecture or science; a performance; a broadcast; a phonogram; a 
compilation of data to the extent it is protected as a copyrightable 
work; or a work performed by a variety or circus performer to the extent 
it is not otherwise considered a literary or artistic work.

"You" means an individual or entity exercising rights under this License 
who has not previously violated the terms of this License with respect 
to the Work, or who has received express permission from the Licensor to 
exercise rights under this License despite a previous violation.

"Publicly Perform" means to perform public recitations of the Work and 
to communicate to the public those public recitations, by any means or 
process, including by wire or wireless means or public digital 
performances; to make available to the public Works in such a way that 
members of the public may access these Works from a place and at a place 
individually chosen by them; to perform the Work to the public by any 
means or process and the communication to the public of the performances 
of the Work, including by public digital performance; to broadcast and 
rebroadcast the Work by any means including signs, sounds or images.

"Reproduce" means to make copies of the Work by any means including 
without limitation by sound or visual recordings and the right of 
fixation and reproducing fixations of the Work, including storage of a 
protected performance or phonogram in digital form or other electronic 
medium.

2. Fair Dealing Rights. Nothing in this License is intended to reduce, 
limit, or restrict any uses free from copyright or rights arising from 
limitations or exceptions that are provided for in connection with the 
copyright protection under copyright law or other applicable laws.

3. License Grant. Subject to the terms and conditions of this License, 
Licensor hereby grants You a worldwide, royalty-free, non-exclusive, 
perpetual (for the duration of the applicable copyright) license to 
exercise the rights in the Work as stated below:

- to Reproduce the Work, to incorporate the Work into one or more 
  Collections, and to Reproduce the Work as incorporated in the 
  Collections;

- to create and Reproduce Adaptations provided that any such Adaptation, 
  including any translation in any medium, takes reasonable steps to 
  clearly label, demarcate or otherwise identify that changes were made to 
  the original Work. For example, a translation could be marked "The 
  original work was translated from English to Spanish," or a modification 
  could indicate "The original work has been modified.";

- to Distribute and Publicly Perform the Work including as incorporated in 
  Collections; and,

- to Distribute and Publicly Perform Adaptations.

For the avoidance of doubt:

Non-waivable Compulsory License Schemes. In those jurisdictions in which 
the right to collect royalties through any statutory or compulsory 
licensing scheme cannot be waived, the Licensor reserves the exclusive 
right to collect such royalties for any exercise by You of the rights 
granted under this License;

Waivable Compulsory License Schemes. In those jurisdictions in which the 
right to collect royalties through any statutory or compulsory licensing 
scheme can be waived, the Licensor waives the exclusive right to collect 
such royalties for any exercise by You of the rights granted under this 
License; and,

Voluntary License Schemes. The Licensor waives the right to collect 
royalties, whether individually or, in the event that the Licensor is a 
member of a collecting society that administers voluntary licensing 
schemes, via that society, from any exercise by You of the rights 
granted under this License.

The above rights may be exercised in all media and formats whether now 
known or hereafter devised. The above rights include the right to make 
such modifications as are technically necessary to exercise the rights 
in other media and formats. Subject to Section 8(f), all rights not 
expressly granted by Licensor are hereby reserved.

4. Restrictions. The license granted in Section 3 above is expressly 
made subject to and limited by the following restrictions:

You may Distribute or Publicly Perform the Work only under the terms of 
this License. You must include a copy of, or the Uniform Resource 
Identifier (URI) for, this License with every copy of the Work You 
Distribute or Publicly Perform. You may not offer or impose any terms on 
the Work that restrict the terms of this License or the ability of the 
recipient of the Work to exercise the rights granted to that recipient 
under the terms of the License. You may not sublicense the Work. You 
must keep intact all notices that refer to this License and to the 
disclaimer of warranties with every copy of the Work You Distribute or 
Publicly Perform. When You Distribute or Publicly Perform the Work, You 
may not impose any effective technological measures on the Work that 
restrict the ability of a recipient of the Work from You to exercise the 
rights granted to that recipient under the terms of the License. This 
Section 4(a) applies to the Work as incorporated in a Collection, but 
this does not require the Collection apart from the Work itself to be 
made subject to the terms of this License. If You create a Collection, 
upon notice from any Licensor You must, to the extent practicable, 
remove from the Collection any credit as required by Section 4(c), as 
requested. If You create an Adaptation, upon notice from any Licensor 
You must, to the extent practicable, remove from the Adaptation any 
credit as required by Section 4(c), as requested.

You may Distribute or Publicly Perform an Adaptation only under the 
terms of: (i) this License; (ii) a later version of this License with 
the same License Elements as this License; (iii) a Creative Commons 
jurisdiction license (either this or a later license version) that 
contains the same License Elements as this License (e.g., 
Attribution-ShareAlike 3.0 US)); (iv) a Creative Commons Compatible 
License. If you license the Adaptation under one of the licenses 
mentioned in (iv), you must comply with the terms of that license. If 
you license the Adaptation under the terms of any of the licenses 
mentioned in (i), (ii) or (iii) (the "Applicable License"), you must 
comply with the terms of the Applicable License generally and the 
following provisions: (I) You must include a copy of, or the URI for, 
the Applicable License with every copy of each Adaptation You Distribute 
or Publicly Perform; (II) You may not offer or impose any terms on the 
Adaptation that restrict the terms of the Applicable License or the 
ability of the recipient of the Adaptation to exercise the rights 
granted to that recipient under the terms of the Applicable License; 
(III) You must keep intact all notices that refer to the Applicable 
License and to the disclaimer of warranties with every copy of the Work 
as included in the Adaptation You Distribute or Publicly Perform; (IV) 
when You Distribute or Publicly Perform the Adaptation, You may not 
impose any effective technological measures on the Adaptation that 
restrict the ability of a recipient of the Adaptation from You to 
exercise the rights granted to that recipient under the terms of the 
Applicable License. This Section 4(b) applies to the Adaptation as 
incorporated in a Collection, but this does not require the Collection 
apart from the Adaptation itself to be made subject to the terms of the 
Applicable License.

If You Distribute, or Publicly Perform the Work or any Adaptations or 
Collections, You must, unless a request has been made pursuant to 
Section 4(a), keep intact all copyright notices for the Work and 
provide, reasonable to the medium or means You are utilizing: (i) the 
name of the Original Author (or pseudonym, if applicable) if supplied, 
and/or if the Original Author and/or Licensor designate another party or 
parties (e.g., a sponsor institute, publishing entity, journal) for 
attribution ("Attribution Parties") in Licensor's copyright notice, 
terms of service or by other reasonable means, the name of such party or 
parties; (ii) the title of the Work if supplied; (iii) to the extent 
reasonably practicable, the URI, if any, that Licensor specifies to be 
associated with the Work, unless such URI does not refer to the 
copyright notice or licensing information for the Work; and (iv) , 
consistent with Ssection 3(b), in the case of an Adaptation, a credit 
identifying the use of the Work in the Adaptation (e.g., "French 
translation of the Work by Original Author," or "Screenplay based on 
original Work by Original Author"). The credit required by this Section 
4(c) may be implemented in any reasonable manner; provided, however, 
that in the case of a Adaptation or Collection, at a minimum such credit 
will appear, if a credit for all contributing authors of the Adaptation 
or Collection appears, then as part of these credits and in a manner at 
least as prominent as the credits for the other contributing authors. 
For the avoidance of doubt, You may only use the credit required by this 
Section for the purpose of attribution in the manner set out above and, 
by exercising Your rights under this License, You may not implicitly or 
explicitly assert or imply any connection with, sponsorship or 
endorsement by the Original Author, Licensor and/or Attribution Parties, 
as appropriate, of You or Your use of the Work, without the separate, 
express prior written permission of the Original Author, Licensor and/or 
Attribution Parties.

Except as otherwise agreed in writing by the Licensor or as may be 
otherwise permitted by applicable law, if You Reproduce, Distribute or 
Publicly Perform the Work either by itself or as part of any Adaptations 
or Collections, You must not distort, mutilate, modify or take other 
derogatory action in relation to the Work which would be prejudicial to 
the Original Author's honor or reputation. Licensor agrees that in those 
jurisdictions (e.g. Japan), in which any exercise of the right granted 
in Section 3(b) of this License (the right to make Adaptations) would be 
deemed to be a distortion, mutilation, modification or other derogatory 
action prejudicial to the Original Author's honor and reputation, the 
Licensor will waive or not assert, as appropriate, this Section, to the 
fullest extent permitted by the applicable national law, to enable You 
to reasonably exercise Your right under Section 3(b) of this License 
(right to make Adaptations) but not otherwise.

5. Representations, Warranties and Disclaimer

UNLESS OTHERWISE MUTUALLY AGREED TO BY THE PARTIES IN WRITING, LICENSOR 
OFFERS THE WORK AS-IS AND MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY 
KIND CONCERNING THE WORK, EXPRESS, IMPLIED, STATUTORY OR OTHERWISE, 
INCLUDING, WITHOUT LIMITATION, WARRANTIES OF TITLE, MERCHANTIBILITY, 
FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT, OR THE ABSENCE OF 
LATENT OR OTHER DEFECTS, ACCURACY, OR THE PRESENCE OF ABSENCE OF ERRORS, 
WHETHER OR NOT DISCOVERABLE. SOME JURISDICTIONS DO NOT ALLOW THE 
EXCLUSION OF IMPLIED WARRANTIES, SO SUCH EXCLUSION MAY NOT APPLY TO YOU.

6. Limitation on Liability. EXCEPT TO THE EXTENT REQUIRED BY APPLICABLE 
LAW, IN NO EVENT WILL LICENSOR BE LIABLE TO YOU ON ANY LEGAL THEORY FOR 
ANY SPECIAL, INCIDENTAL, CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES 
ARISING OUT OF THIS LICENSE OR THE USE OF THE WORK, EVEN IF LICENSOR HAS 
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

7. Termination

This License and the rights granted hereunder will terminate 
automatically upon any breach by You of the terms of this License. 
Individuals or entities who have received Adaptations or Collections 
from You under this License, however, will not have their licenses 
terminated provided such individuals or entities remain in full 
compliance with those licenses. Sections 1, 2, 5, 6, 7, and 8 will 
survive any termination of this License. Subject to the above terms and 
conditions, the license granted here is perpetual (for the duration of 
the applicable copyright in the Work). Notwithstanding the above, 
Licensor reserves the right to release the Work under different license 
terms or to stop distributing the Work at any time; provided, however 
that any such election will not serve to withdraw this License (or any 
other license that has been, or is required to be, granted under the 
terms of this License), and this License will continue in full force and 
effect unless terminated as stated above. 8. Miscellaneous

Each time You Distribute or Publicly Perform the Work or a Collection, 
the Licensor offers to the recipient a license to the Work on the same 
terms and conditions as the license granted to You under this License.

Each time You Distribute or Publicly Perform an Adaptation, Licensor 
offers to the recipient a license to the original Work on the same terms 
and conditions as the license granted to You under this License.

If any provision of this License is invalid or unenforceable under 
applicable law, it shall not affect the validity or enforceability of 
the remainder of the terms of this License, and without further action 
by the parties to this agreement, such provision shall be reformed to 
the minimum extent necessary to make such provision valid and 
enforceable.

No term or provision of this License shall be deemed waived and no 
breach consented to unless such waiver or consent shall be in writing 
and signed by the party to be charged with such waiver or consent.

This License constitutes the entire agreement between the parties with 
respect to the Work licensed here. There are no understandings, 
agreements or representations with respect to the Work not specified 
here. Licensor shall not be bound by any additional provisions that may 
appear in any communication from You. This License may not be modified 
without the mutual written agreement of the Licensor and You.

The rights granted under, and the subject matter referenced, in this 
License were drafted utilizing the terminology of the Berne Convention 
for the Protection of Literary and Artistic Works (as amended on 
September 28, 1979), the Rome Convention of 1961, the WIPO Copyright 
Treaty of 1996, the WIPO Performances and Phonograms Treaty of 1996 and 
the Universal Copyright Convention (as revised on July 24, 1971). These 
rights and subject matter take effect in the relevant jurisdiction in 
which the License terms are sought to be enforced according to the 
corresponding provisions of the implementation of those treaty 
provisions in the applicable national law. If the standard suite of 
rights granted under applicable copyright law includes additional rights 
not granted under this License, such additional rights are deemed to be 
included in the License; this License is not intended to restrict the 
license of any rights under applicable law.

