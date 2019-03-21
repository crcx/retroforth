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

The RETRO source code can be obtained from http://forthworks.com/retro
or gopher://forthworks.com/1/retro

## Stable Releases

I periodically make stable releases. This will typically happen
two to four times per year. These are good for those needing a
solid base that doesn't change frequently.

## Snapshots

A lot of development happens between releases. I make snapshots
of my working source tree nightly (and often more often).

This is what I personally recommend for most users. It reflects
my latest system and is normally reliable as it's used daily in
production.

The latest snapshot can be downloaded from the following stable
URLs:

* http://forthworks.com/retro/r/latest.tar.gz
* gopher://forthworks.com/9/retro/r/latest.tar.gz

## Repository

I use a Fossil repository to manage development. To obtain a
copy of the repository install Fossil and:

```
fossil clone http://forthworks.com:8000 retro.fossil
mkdir retro
cd retro
fossil open /path/to/retro.fossil
```

See the Fossil documentation for details on using Fossil to
keep your local copy of the repository current.

This will let you stay current with my latest changes faster
than the snapshots, but you may occasionally encounter bigger
problems as some commits may be in a partially broken state.

If you have problems, check the version of Fossil you are
using. I am currently using Fossil 2.7, you may experience
issues checking out or cloning if using older versions.

# Building RETRO on BSD

RETRO is well supported on BSD (FreeBSD, NetBSD, OpenBSD)
systems. It should build on a base install of any of these
without issue.

## Requirements

- c compiler
- make

## Process

Run `make`

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

# Building RETRO on Linux

Building on Linux is pretty easy. You'll need to make sure
you have a C compiler, headers, and make.

## Requirements

- c compiler (tested: clang, tcc, gcc)
- development headers
- make

## Process

Run:

```
make -f Makefile.linux
```

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

# Building RETRO on macOS

To build on macOS, you will need the command line tools from
Xcode. Install these and you should be able to build and use
RETRO.

## Requirements

- command line tools from Xcode

## Process

Run `make`

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

It is possible to build RETRO on Windows, though a few of the
extensions are not supported:

- no `unix:` words
- no `gopher:` words

## Process

This is currently more difficult than on a Unix host. If you have
Windows 10 and WSL, it may be better to build under that (using
the Linux instructions).

### Setup TCC

Go to http://download.savannah.gnu.org/releases/tinycc/

Download the *winapi-full* and *tcc-xxxx-bin* packages for your
system. Decompress them, copy the headers from the winapi
package into the tcc directory.

### Prepare Source

You'll need to comment out (or remove) some things before RETRO
will build.

In *rre.c*:

- remove includes for unistd.h, sys/sockets.h, netinet/in.h
  netdb.h, errno.h, sys/wait.h, signal.h
- remove the #define USE_TERMIOS line
- change the #define NUM_DEVICES to 6
- remove io_unix_handler and io_gopher_handler from IO_deviceHandlers
- remove io_unix_query and io_gopher_query from IO_queryHandlers

In *image-functions.c*:

- remove includes for unistd.h

In *image-functions.h*:

- remove includes for unistd.h

In *io\io_filesystem.c*:

- remove includes for unistd.h

In *io\io_floatingpoint.c*:

- remove includes for unistd.h

### Build

\path\to\tcc rre.c image-functions.c io\io_filesystem.c io\io_floatingpoint.c -o retro.exe

# Starting RETRO on BSD

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


# Starting RETRO on Linux

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


# Starting RETRO on macOS

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


# Starting RETRO on Windows

Double click the `retro.exe` file.

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
:palindrome dup s:reverse s:eq? ;
```

Naming uses the `:` prefix to add a new word to the dictionary.
The words that make up the definition are then placed, with a
final word (`;`) ending the definition. We can then use this:

```
'anna palindrome?
```

Once defined there is no difference between our new word and
any of the words already provided by the RETRO system.

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

## General Guidelines

* Readability is important
* Be consistent
* Don't use a prefix as the first character of a name
* Use short names for indices

## Typical Format

The word names will generally follow a form like:

    [namespace:]name

The `namespace:` is optional, but recommended for consistency
with the rest of the system and to make it easier to identify
related words.

## Case

Word names are lowercase, with a dash (-) for compound names.

```
hello
drop-pair
s:for-each
```

Variables use TitleCase, with no dash between compound names.

```
Base
Heap
StringBuffers
```

Constants are UPPERCASE, with a dash (-) for compound names.

```
TRUE
FALSE
f:PI
MAX-STRING-LENGTH
```

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

## Tips

Avoid using a prefix as the first character of a word name. RETRO
will look for prefixes first, this will prevent direct use of
the work in question.

To find a list of prefix characters, do:

```
'prefix: d:words-with
```

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

There is also a `times<with-index>` variation that provides
access to the loop index (via `I`) and parent loop indexes
(via `J` and `K`).

```
#10 [ I n:put sp ] times<with-index>
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


# Working With Arrays

RETRO offers a number of words for operating on statically sized
arrays.

## Namespace

The words operating on arrays are kept in an `array:` namespace.

## Creating Arrays

The easiest way to create an array is to wrap the values in a
`{` and `}` pair:

```
{ #1 #2 #3 #4 }
{ 'this 'is 'an 'array 'of 'strings }
{ 'this 'is 'a 'mixed 'array #1 #2 #3 }
```

You can also make an array from a quotation which returns
values and the number of values to store in the array:

```
[ #1 #2 #3  #3 ] array:counted-results
[ #1 #2 #3  #3 ] array:make
```

## Accessing Elements

You can access a specific value with `array:nth` and `fetch` or
`store`:

```
{ #1 #2 #3 #4 } #3 array:nth fetch
```

## Find The Length

Use `array:length` to find the size of the array.

```
{ #1 #2 #3 #4 } array:length
```

## Duplicate

Use `array:dup` to make a copy of an array:

```
{ #1 #2 #3 #4 } array:dup
```

## Filtering

RETRO provides `array:filter` which extracts matching values
from an array. This is used like:

```
{ #1 #2 #3 #4 #5 #6 #7 #8 } [ n:even? ] array:filter
```

The quote will be passed each value in the array and should
return TRUE or FALSE. Values that lead to TRUE will be collected
into a new array.

## Mapping

`array:map` applies a quotation to each item in an array and
constructs a new array from the returned values.

Example:

```
{ #1 #2 #3 } [ #10 * ] array:map
```

## Reduce

`array:reduce` takes an array, a starting value, and a quote. It
executes the quote once for each item in the array, passing the
item and the value to the quote. The quote should consume both
and return a new value.

```
{ #1 #2 #3 } #0 [ + ] array:reduce
```

## Search

RETRO provides `array:contains?` and `array:contains-string?`
to search an array for a value (either a number or string) and
return either TRUE or FALSE.

```
#100  { #1 #2 #3 } array:contains?
'test { 'abc 'def 'test 'ghi } array:contains-string?
```

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

You can use `s:append` or `s:prepend` to merge two strings.

```
'First 'Second s:append
'Second 'First s:prepend
```

## Tokenization

`s:tokenize`
`s:tokenize-on-string`
`s:split`
`s:split-on-string`

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
    | 0 - 1024        | rx kernel                    |
    | 1025 - 1535     | token input buffer           |
    | 1536 +          | start of heap space          |
    | ............... | free memory for your use     |
    | 506879          | buffer for string evaluate   |
    | 507904          | temporary strings (32 * 512) |
    | 524287          | end of memory                |

The buffers at the end of memory will resize when specific
variables related to them are altered.

# Additional Tools

In addition to the core `retro` binary, the `bin` directory
will contain a few other tools.

## retro

This is the main RETRO binary.

## retro-describe

This is a program that looks up entries in the Glossary.

At the command line, you can use it like:

```
retro-describe s:for-each
```

## retro-embedimage

This is a program which generates a C file with the ngaImage
contents. It's used when building `retro`.

```
retro-embedimage ngaImage
```

The output is written to stdout; redirect it as needed.

## retro-extend

This is a program which compiles code into the ngaImage.
It's used when building `retro` and when you want to make a
standalone image with custom additions.

Example command line:

```
retro-extend ngaImage example/rot13.forth
```

Pass the image name as the first argument, and then file names
as susequent ones. Do *not* use this for things relying on I/O
apart from the basic console output as it doesn't emulate other
devices. If you need to load in things that rely on using the
optional I/O devices, see the Advanced Builds chapter.

## retro-muri

This is the assembler for Nga. It's used to build the initial
RETRO kernel and can be used by other tools as well.

## retro-unu

This is the literate source extraction tool for RETRO. It
is used in building `retro`.

Example usage:

```
retro-unu literate/RetroForth.md
```

Output is written to stdout; redirect as neeeded.

# Advanced Builds

For users of BSD, Linux, macOS, you can customize the image at
build time.

In the top level directory is a `package` directory containing
a file named `list`. You can add files to compile into your
system by adding them to the `list` and rebuilding.

Example:

If you have wanted to include the NumbersWithoutPrefixes.forth
example, add:

    ~~~
    'example/NumbersWithoutPrefixes.forth include
    ~~~

To the start of the `list` file and then run `make` again. The
newly built `bin/retro` will now include your additions.

# The Optional Retro Compiler

In addition to the base system, users of RETRO on Unix hosts
with ELF executables can build and use the `retro-compiler`
to generate turnkey executables.

## Requirements

- Unix host
- ELF executable support
- objcpy in the $PATH

## Building

BSD users:

   make bin/retro-compiler

Linux users:

   make -f Makefile.linux bin/retro-compiler

## Installing

Copy `bin/retro-compiler` to somewhere in your $PATH.

## Using

`retro-compiler` takes two arguments: the source file to
compile and the name of the word to use as the main entry
point.

Example:

Given a `hello.forth`:

```
:hello 'Hello_World! s:put nl ;
```

Use:

```
retro-compiler hello.forth hello
```

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


# Technical Notes and Reflections

This is a collection of short papers providing some additional
background and reflections on design decisions.

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

In implementing the RETRO 12 kernel (called Rx) I had to decide
on what functionality would be needed. It was important to me
that this be kept clean and minimalistic, as I didn't want to
spend a lot of time changing it as time progressed. It's far
nicer to code at the higher level, where the RETRO language is
functional, as opposed to writing more assembly code.

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

I *could* slightly reduce this. The $ prefix could be defined in
higher level code, and I don't strictly *need* to expose the
`fetch-next` and `store-next` here. But since the are already
implemented as dependencies of the words in the kernel, it would
be a bit wasteful to redefine them later in higher level code.

With these words the rest of the language can be built up. Note
that the Rx kernel does not provide any I/O words. It's assumed
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

