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

RETRO is well supported on BSD (FreeBSD, NetBSD, OpenBSD)
systems. It should build on a base install of any of these
without issue.

## Requirements

- c compiler
- make

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
    retro-describe

# Building RETRO on Linux

Building on Linux is pretty easy. You'll need to make sure
you have a C compiler, headers, and make.

## Requirements

- c compiler (tested: clang, tcc, gcc)
- development headers
- make

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
    retro-describe

# Building RETRO on macOS

To build on macOS, you will need the command line tools from
Xcode. Install these and you should be able to build and use
RETRO.

## Requirements

- command line tools from Xcode

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
with it.

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

### Compositional

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

### Execution Flow

Combinators of this type execute other functions.

#### Fundamental

`call` takes a quote and executes it immediately.

```
[ #1 n:put ] call
&words call
```

#### Conditionals

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

#### Looping

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

### Data Flow

These combinators exist to simplify stack usage in various
circumstances.

#### Preserving

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

#### Cleave

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

#### Spread

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

#### Apply

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
{ #1 #2 #3 #4 #5 #6 #7 #8 }
[ n:even? ] array:filter
```

The quote will be passed each value in the array and should
return TRUE or FALSE. Values that lead to TRUE will be collected
into a new array.

## Mapping

`array:map` applies a quotation to each item in an array and
constructs a new array from the returned values.

Example:

```
{ #1 #2 #3 }
[ #10 * ] array:map
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

Words operating on the buffer are kept in the `buffer`
namespace.

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

In the top level directory is a `packages` directory containing
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

