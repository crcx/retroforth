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

# Building RETRO on BSD

## Requirements

- c compiler (tested: clang, tcc, gcc)
- make
- standard unix shell

## Process

Run `make`.

This will build the toolchain and then the main `retro`
executable.

## Executables

In the `bin/` directory:

    retro
    retro-unu
    retro-muri
    retro-extend
    retro-embedimage


# Building RETRO on Linux

## Requirements

- c compiler (tested: clang, tcc, gcc)
- make
- standard unix shell

## Process

Run `make -f Makefile.linux`.

This will build the toolchain and then the main `retro`
executable.

## Executables

In the `bin/` directory:

    retro
    retro-unu
    retro-muri
    retro-extend
    retro-embedimage


# Building RETRO on macOS

## Requirements

- c compiler (tested: clang, tcc, gcc)
- make
- standard unix shell

## Process

Run `make`.

This will build the toolchain and then the main `retro`
executable.

## Executables

In the `bin/` directory:

    retro
    retro-unu
    retro-muri
    retro-extend
    retro-embedimage


# Building RETRO on Windows

## C#: retro.cs

This is an implementation of `retro-repl` in C#. As with
`retro-repl` it requires the `ngaImage` in the current
directory when starting.

Building:

    csc retro.cs

You'll need to make sure your path has the CSC.EXE in it,
or provide a full path to it. Something like this should
reveal the path to use:

    dir /s %WINDIR%\CSC.EXE

I've only tested building this using Microsoft's .NET tools.
It should also build and run under Mono.


# Starting RETRO

RETRO can be run for scripting or interactive use. To start it
interactively, run: `retro -i` or `retro -c`.

For a summary of the full command line arguments available:

    Scripting Usage:

        retro filename [script arguments...]

    Interactive Usage:

        retro [-h] [-i] [-c] [-s] [-f filename] [-t]

      -h           Display this help text
      -i           Interactive mode (line buffered)
      -c           Interactive mode (character buffered)
      -s           Suppress the 'ok' prompt and keyboard
                   echo in interactive mode
      -f filename  Run the contents of the specified file
      -t           Run tests (in ``` blocks) in any loaded files


# Starting RETRO

RETRO can be run for scripting or interactive use. To start it
interactively, run: `retro -i` or `retro -c`.

For a summary of the full command line arguments available:

    Scripting Usage:

        retro filename [script arguments...]

    Interactive Usage:

        retro [-h] [-i] [-c] [-s] [-f filename] [-t]

      -h           Display this help text
      -i           Interactive mode (line buffered)
      -c           Interactive mode (character buffered)
      -s           Suppress the 'ok' prompt and keyboard
                   echo in interactive mode
      -f filename  Run the contents of the specified file
      -t           Run tests (in ``` blocks) in any loaded files


# Starting RETRO

RETRO can be run for scripting or interactive use. To start it
interactively, run: `retro -i` or `retro -c`.

For a summary of the full command line arguments available:

    Scripting Usage:

        retro filename [script arguments...]

    Interactive Usage:

        retro [-h] [-i] [-c] [-s] [-f filename] [-t]

      -h           Display this help text
      -i           Interactive mode (line buffered)
      -c           Interactive mode (character buffered)
      -s           Suppress the 'ok' prompt and keyboard
                   echo in interactive mode
      -f filename  Run the contents of the specified file
      -t           Run tests (in ``` blocks) in any loaded files



# Basic Interactions

Start RETRO in interactive mode:

```
retro -i
```

You should see something similar to this:

    RETRO 12 (rx-2019.6)
    8388608 MAX, TIB @ 1025, Heap @ 9374

    Ok

At this point you are at the *listener*, which reads and
processes your input. You are now set to begin exploring
RETRO.

To exit, run `bye`:

```
bye
```


# Using The Glossary

The Glossary is a valuable resource. It provides information
on the RETRO words.

## Example Entry

    f:+

      Data:  -
      Addr:  -
      Float: FF-F

    Add two floating point numbers, returning the result.

    Class: class:word | Namespace: f | Interface Layer: rre

    Example #1:

        .3.1 .22 f:+

## Reading The Entry

An entry starts with the word name.

This is followed by the stack effect for each stack. All RETRO
systems have Data and Address stacks, some also include a
floating point stack).

The stack effect diagrams are followed by a short description
of the word.

After the description is a line providing some useful data. This
includes the class handler, namespace prefix, and the interface
layer that provides the word.

Words in all systems will be listed as `all`. Some words (like
the `pb:` words) are only on specific systems like iOS. These
can be identified by looking at the interface layer field.

At the end of the entry may be an example or two.

## Access Online

The latest Glossary can be browsed at http://forthworks.com:9999
or gopher://forthworks.com:9999

# Programming Techniques

The upcoming chapters provide helpful information on using RETRO
with different types of data and hints on how to solve problems
in a way consistent with the RETRO system.

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

(Note: this only applies to *source files*; fences are not used
when entering code interactively).

# Naming Conventions

Word names in RETRO generally follow the following conventions.

## Case

Word names are lowercase, with a dash (-) for compound names.
Variables use TitleCase, with no dash between compound names.
Constants are UPPERCASE, with a dash (-) for compound names.

## Namespaces

Words are grouped into broad namespaces by attaching a short
prefix string to the start of a name.

The common namespaces are:

| Prefix  | Contains                                               |
| ------- | ------------------------------------------------------ |
| array:  | Words operating on simple arrays                       |
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
| gopher: | Gopher protocol words                                  |
| unix:   | Unix system call words                                 |

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

    b, n, m, o, x, y, z are generic numeric values
    s represents a string
    v represents a variable
    p, a represent pointers
    q represents a quotation
    d represents a dictionary header
    f represents a `TRUE` or `FALSE` flag.

In the case of something like `(xyz-m)`, RETRO expects z to be
on the top of the stack, with y below it and x below the y
value. And after execution, a single value (m) will be left on
the stack.

Words with no stack effect have a comment of (-)





# Working With a Buffer

RETRO provides words for operating on a linear memory area.
This can be useful in building strings or custom data
structures.

## Namespace

Words operating on the buffer are kept in the `buffer`
namespace.

# Working With Characters

RETRO provides words for working with ASCII characters.

## Prefix

Character constants are returned using the `$` prefix.

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

`d:last` returns a pointer to the latest header. `d:last<xt>`
will give the contents of the `d:xt` field for the latest
header. There are also `d:last<class>` and `d:last<name>`.

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

# Working With Numbers

Numbers in RETRO are signed, 32 bit integers with a range of
-2,147,483,648 to 2,147,483,647.

## Token Prefix

All numbers start with a `#` prefix.

## Namespace

Most words operating on numbers are in the `n:` namespace.



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

```
'Hello
'This_is_a_string
'This_is_a_much_longer_string_12345_67890_!!!
```

RETRO will replace spaces with underscores. If you need both
spaces and underscores in a string, escape the underscores and
use `s:format`:

```
'This_has_spaces_and_under\_scored_words. s:format
```

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

RETRO provides two words for searching within a string.

`s:contains-char?` 
`s:contains-string?`
`s:index-of`
`s:index-of-string`

## Comparisons

`s:eq?`
`s:case`

## Extraction

`s:left`
`s:right`
`s:substr`

## Joining

`s:append`
`s:prepend`

## Tokenization

`s:tokenize`
`s:tokenize-on-string`
`s:split`
`s:split-on-string`

## Conversions

`s:to-lower`
`s:to-upper`
`s:to-number`

## Cleanup

`s:chop`
`s:trim`
`s:trim-left`
`s:trim-right`

## Combinators

`s:for-each`
`s:filter`
`s:map`

## Other

`s:evaluate`
`s:copy`
`s:reverse`
`s:hash`
`s:length`
`s:replace`
`s:format`
`s:empty`


# The Return Stack

RETRO has two stacks. The primary one is used to pass data
beween words. The second one primarily holds return addresses.

Each time a word is called, the next address is pushed to
the return stack.

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
    1 lit   6 pop   11 eq    16 store 21 and   26 end
    2 dup   7 jump  12 neq   17 add   22 or    27 ienum
    3 drop  8 call  13 lt    18 sub   23 xor   28 iquery
    4 swap  9 ccall 14 gt    19 mul   24 shift 29 iinvoke

This reduces to:

    0 ..    5 pu    10 re    15 fe    20 di    25 zr
    1 li    6 po    11 eq    16 st    21 an    26 en
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

    :n:square as{ 'dumure.. i }as ;

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

# Internals

The next few chapters dive into RETRO's architecture. If you
seek to implement a port to a new platform or to extend the
I/O functionality you'll find helpful information here.

# Internals: Nga Virtual Machine

## Overview

At the heart of RETRO is a simple MISC (minimal instruction
set computer) processor for a dual stack architecture.

This is a very simple and straightforward system. There are
30 instructions. The memory is a linear array of signed 32
bit values. And there are two stacks: one for data and one
for return addresses.

## Instrution Table

Column:

  0 - opcode value
  1 - Muri assembly name
  2 - Full name
  3 - Data Stack Usage
  4 - Address Stack Usage

    +--------------------------------------------------+
    |  0 ..  nop                   -           -       |
    |  1 li  lit                   -n          -       |
    |  2 du  dup                  n-nn         -       |
    |  3 dr  drop                 n-           -       |
    |  4 sw  swap                xy-yx         -       |
    |  5 pu  push                 n-           -n      |
    |  6 po  pop                   -n         n-       |
    |  7 ju  jump                 a-           -       |
    |  8 ca  call                 a-           -A      |
    |  9 cc  conditional call    af-           -A      |
    | 10 re  return                -          A-       |
    | 11 eq  equality            xy-f          -       |
    | 12 ne  inequality          xy-f          -       |
    | 13 lt  less than           xy-f          -       |
    | 14 gt  greater than        xy-f          -       |
    | 15 fe  fetch                a-n          -       |
    | 16 st  store               na-           -       |
    | 17 ad  addition            xy-n          -       |
    | 18 su  subtraction         xy-n          -       |
    | 19 mu  multiplication      xy-n          -       |
    | 20 di  divide & remainder  xy-rq         -       |
    | 21 an  bitwise and         xy-n          -       |
    | 22 or  bitwise or          xy-n          -       |
    | 23 xo  bitwise xor         xy-n          -       |
    | 24 sh  shift               xy-n          -       |
    | 25 zr  zero return          n-n | n-     -       |
    | 26 en  end                   -           -       |
    | 27 ie  i/o enumerate         -n          -       |
    | 28 iq  i/o query            n-xy         -       |
    | 29 ii  i/o invoke         ...n-          -       |
    |                                                  |
    | Each `li` will push the value in the following   |
    | cell to the data stack.                          |
    +--------------------------------------------------+
    |             li       du       mu       ..        |
    | i lidumu..  00000001:00000010:00010011:00000000  |
    |             data for li                          |
    | d 2         00000000:00000000:00000000:00000010  |
    |                                                  |
    | Assembler Directives        Instruction Bundles  |
    | ========================    ==================== |
    | : label                     Combine instruction  |
    | i bundle                    names in groups of 4 |
    | d numeric-data                                   |
    | r ref-to-address-by-name    Use only .. after    |
    | s null-terminated string    ju, ca, cc, re, zr   |
    +--------------------------------------------------+

## Misc

There are 810,000 possible combinations of instructions. Only
73 are used in the implementation of RETRO.

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



# Internals: The Retro Image

The actual RETRO language is stored as a memory image for Nga.

## Layout

Assuming an Nga built with 524287 cells of memory:

| RANGE           | CONTAINS                     |
| --------------- | ---------------------------- |
| 0 - 1024        | rx kernel                    |
| 1025 - 1535     | token input buffer           |
| 1536 +          | start of heap space          |
| ............... | free memory for your use     |
| 506879          | buffer for string evaluate   |
| 507904          | temporary strings (32 * 512) |
| 524287          | end of memory                |

The buffers at the end of memory will resize when specific
variables related to them are altered.




