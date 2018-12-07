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

This book is structured into topics.

---

# Quick Start

## Get RETRO

RETRO can be obtained at:

- [https://forthworks.com/retro](https://forthworks.com/retro)
- [gopher://forthworks.com/1/retro](gopher://forthworks.com/1/retro)

### Build From Source

Run `make` to compile RETRO and its toolchain. The binaries will
be placed in the `bin` directory.

If the build fails, try `make -f Makefile.alternate`.

There are various binaries. The main one is `retro`. Copy this
to a place in your $PATH and you'll be set to run it.

### Use A Prebuilt Binary

Go to forthworks.com/retro and download the binaries collection.
Copy the appropriate binary to a location in your path.

### Use An OS Package

RETRO is packaged for a few systems.

TODO: add details here.

### Commercial Versions

#### iOS

#### macOS

## Run RETRO

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

---

# The Virtual Machine

RETRO runs on a virtual machine called Nga. This emulates a MISC
(minimal instruction set computer), with 30 instructions and two
stacks.

Memory is provided as a linear array of signed 32-bit values.
There is no direct addressing of other values.

## Instruction Set

### 00 NOP

**NOP** does nothing. It's used for padding and reserving space.

### 01 LIT

**LIT** pushes the value in the following cell to the data
stack. This is the only instruction which takes a value from
a source other than the stack.

### 02 DUP

**DUP** duplicates the top value on the stack.

### 03 DROP

**DROP** discards the top item on the stack.

### 04 SWAP

**SWAP** switches the positions of the top two items on the
stack.

### 05 PUSH

**PUSH** moves a value from the data stack to the address stack.

### 06 POP

**POP** moves a value from the address stack to the data stack.

### 07 JUMP

**JUMP** takes an address from the stack and transfers control
to the code at the specified address.

### 08 CALL

**CALL** calls a subroutine at the address on the top of the
stack.

### 09 CCALL

**CCALL** is a conditional call. It takes two values: a flag and
a pointer for an address to jump to if the flag is true.

At the instruction level, a false flag is zero and any other
value is true.

### 10 RETURN

**RETURN** ends a subroutine and returns flow to the instruction
following the last **CALL** or **CCALL**.


### 11 EQ

**EQ** compares two values for equality and returns a flag.

### 12 NEQ

**NEQ** compares two values for inequality and returns a flag.

### 13 LT

**LT** compares two values for less than and returns a flag.

### 14 GT

**GT** compares two values for greater than and returns a flag.

### 15 FETCH

**FETCH** takes an address and returns the value stored there.

This doubles as a means of introspection into the VM state.
Negative addresses correspond to VM queries:

    | Address | Returns             |
    | ------- | ------------------- |
    | -1      | Data stack depth    |
    | -2      | Address stack depth |
    | -3      | Maximum Image Size  |

An implementation may use negative values below -100 for
implementation specific inquiries.

### 16 STORE

**STORE** stores a value into an address.

### 17 ADD

**ADD** adds two numbers together.

### 18 SUB

**SUB** subtracts two numbers.

### 19 MUL

**MUL** multiplies two numbers.

### 20 DIVMOD

**DIVMOD** divides and returns the quotient and remainder.

### 21 AND

**AND** performs a bitwise AND operation.

    +----------------+
    | Before | After |
    | ------ | ----- |
    | -1     | -1    |
    | -1     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    |  0     |  0    |
    | -1     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    | -1     |  0    |
    |  0     |       |
    +----------------+

### 22 OR

**OR** performs a bitwise OR operation.

    +----------------+
    | Before | After |
    | ------ | ----- |
    | -1     | -1    |
    | -1     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    | -1     | -1    |
    |  0     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    |  0     | -1    |
    | -1     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    |  0     |  0    |
    |  0     |       |
    +----------------+

### 23 XOR

**XOR** performs a bitwise XOR operation.

    +----------------+
    | Before | After |
    | ------ | ----- |
    | -1     |  0    |
    | -1     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    |  0     | -1    |
    | -1     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    | -1     | -1    |
    |  0     |       |
    +----------------+

    +----------------+
    | Before | After |
    | ------ | ----- |
    |  0     |  0    |
    |  0     |       |
    +----------------+

### 24 SHIFT

**SHIFT** performs a bitwise arithmetic SHIFT operation.

This takes two values:

    xy

And returns a single one:

    z

If `y` is positive, this shifts right. If negative, it shifts
left.

### 25 ZRET

**ZRET** returns from a subroutine if the top item on the stack
is zero. If not, it acts like a **NOP** instead.

### 26 END

**END** tells the VM to shut down.

### 27 IENUMERATE

**IENUMERATE** pushes the number of simulated I/O devices to the
stack.

### 28 IQUERY

**IQUERY** takes a device number from the stack and queries it,
returning two values: a device version number and a device type
identifier.

### 29 IINTERACT

**IINTERACT** takes a device number from the stack and triggers
an interaction with it.

## Instruction Encoding

Nga allows up to four instructions to be packed into a single
memory cell. These are decoded and executed sequentially.

    first    second   third   fourth
    00000000 00000000 0000000 0000000

Any unused instructions should be replaced with NOP (00000000).
If an instruction modifies the instruction pointer all later
instructions should be NOP. This is because the later slots
will be executed prior to the IP change. Consider:

    LIT CALL LIT CALL

In this case only the second CALL actually gets control. The
instructions which modify IP are:

    JUMP
    CALL
    CCALL
    RETURN
    ZRET

The LIT instruction takes a value from the following cell
and pushes it to the stack. Multiple LIT's will use sequential
cells. E.g.,

    LIT LIT ADD NOP
    100
    200

---

# Muri Assembly

The initial kernel is written in Nga assembly and is built using
an assembler named **Muri**. This is a simple, multipass model
that's not fancy, but suffices for RETRO's needs.

A small example:

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

    0  nop     7  jump    14  gt      21  and    28  io query
    1  lit     8  call    15  fetch   22  or     29  io interact
    2  dup     9  ccall   16  store   23  xor
    3  drop   10  return  17  add     24  shift
    4  swap   11  eq      18  sub     25  zret
    5  push   12  neq     19  mul     26  end
    6  pop    13  lt      20  divmod  27  io enumerate

This boils down to:

    0  ..    7  ju   14  gt   21  an   28  iq
    1  li    8  ca   15  fe   22  or   29  ii
    2  du    9  cc   16  st   23  xo
    3  dr   10  re   17  ad   24  sh
    4  sw   11  eq   18  su   25  zr
    5  pu   12  ne   19  mu   26  en
    6  po   13  lt   20  di   27  ie

Most are just the first two letters of the instruction name. I
use `..` instead of `no` for `NOP`, and the first letter of
each I/O instruction name. So a bundle may look like:

    dumure..

(This would correspond to `dup multiply return nop`).

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

---

# Source Format

RETRO is literate. Most of the sources are in a format called
Unu. This allows easy mixing of commentary and code blocks,
making it simple to document the code.

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

---

# Syntax

Forth is largely freeform, and RETRO generally follows this. But
there are a few things that impact this.

First, RETRO processes code as encountered, token by token. A
standard system does not expose the parser to the user.

Secondly, RETRO uses single character prefixes to guide the
processing of a token.

The interpreter takes a look at the first character of a token.
If this matches a known *prefix*, the rest of the token is
passed to a function which handles that group of tokens. If no
valid prefix is found, RETRO tries to find the word in the
dictionary. If successful, the information in the *dictionary
header* is used to carry out the actions specified in the name's
*definition*. If this also fails, RETRO calls `err:notfound` to
report the error.

A flowchart of the interpreter process:

    +-------------+
    | Get a Token |
    +-------------+
           |
    +------------------------------------+
    | Is first character a valid prefix? |
    +------------------------------------+
                |                     |
               YES                   NO
                |                    |
    +-----------------------+   +------------------------+
    | Pass rest of token to |   | Is entire token a name |
    | prefix handler        |   | in the dictionary?     |
    +-----------------------+   +------------------------+
                                    |                |
                                   YES              NO
                                    |               |
                    +------------------+   +-------------------+
                    | Push XT to stack |   | Call err:notfound |
                    +------------------+   +-------------------+
                              |
                   +--------------------+
                   | Call class handler |
                   +--------------------+

In RETRO, *prefix handlers* and *class handlers* are responsible
for dealing with the tokens and words. This includes interpret
and compilation as necessary.

RETRO permits names to contain any characters other than space,
tab, cr, dnd lf. Names are *case sensitive*, so the following
are three *different* names:

    foo Foo FOO

I recommend that names not start with a *prefix* as prefixes
are checked prior to dictionary lookups. So a word named `@foo`
would be treated as a fetch from a variable named `foo` instead
of running the `@foo` word.

Tip: Run `'prefix: d:words-with` to get a list of prefix
     handlers in your system.

---

# The Compiler

To create new functions, you use the compiler. This is generally
started by using the `:` (pronounced *colon*) prefix. A simple
example:

    :foo #1 #2 + n:put ;

Breaking this apart:

    :foo

RETRO sees that `:` is a prefix and calls the handler for it.
The handler creates a new word named *foo* and starts the
compiler.

    #1

RETRO sees that `#` is a prefix and calls the handler for it.
The handler converts the token to a number and then pushes the
value to the stack. Since the `Compiler` is now active, it then
pops the value off the stack and compiles it into the current
definition.

    #2

And again, but compile a *2* instead of a *1*.

   +

RETRO does not find a `+` prefix, so it searches the dictionary.
Upon finding `+`, it pushes the address (*xt*) to the stack and
calls the corresponding class handler. The class handler for
normal words calls the code at the address if interpreting, or
compiles a call to it if the `Compiler` is active.

    n:put

The process is repeated for `n:put`.

    ;

The last word has a slight difference. Like `+` and `n:put`,
this is a word, not a prefixed token. But the class handler
for this one always calls the associated code. In this case,
`;` is the word which ends a definition and turns off the
`Compiler`.

---

# Hyperstatic Global Environment

This now brings up an interesting subpoint. RETRO provides
what is sometimes called a *hyper-static global environment.*
This can be difficult to explain, so let's take a quick look
at how it works:

    :scale (x-y) A fetch * ;
    >>> A ?
    #1000 'A var<n>
    :scale (x-y) A fetch * ;
    #3 scale n:put
    >>> 3000
    #100 A store
    #3 scale n:put
    >>> 300
    #5 'A var<n>
    #3 scale n:put
    >>> 300
    A fetch n:put
    >>> 5

Output is marked with **>>>**.

Note that we create two variables with the same name (*A*). The
definition for *scale* still refers to the old variable, even
though we can no longer directly manipulate it.

In a hyper-static global environment, functions continue to
refer to the variables and functions that existed when they were
defined. If you create a new variable or function with the same
name as an existing one, it only affects future code.

Take advantage of this to reuse short names whenever it helps to
make your code easier to understand.

---

# Classes

The interpreter is unaware of how to handle a dictionary entry
and has no concept of the difference between compiling and
interpreting.

The actual work is handled by something I call *class handlers*.

Each dictionary header contains a variety of information:

    +--------+------------------+
    | Offset | Description      |
    +========+==================+
    | 0      | link to previous |
    +--------+------------------+
    | 1      | class handler    |
    +--------+------------------+
    | 2      | xt               |
    +--------+------------------+
    | 3+     | name of function |
    +--------+------------------+

When a token is found, the listener pushes the contents of the
xt field to the stack and then calls the function that the class
handler field points to.

This model differs from traditional Forth since the interpreter
is unaware of how tokens are handled. All actions are performed
by the class handlers, which allows for easy addition of new
categories of words and functionality.

---

# Data Structures

You can create named data structures using `d:create`, `var`,
`var<n>`, and `const`.

## Constants

These are the simplest data structure. The *xt* is set to a
value, which is either left on the stack or compiled into a
definition.

    #100 'ONE-HUNDRED const

By convention, constants in RETRO should have names in all
uppercase.

## Variables

A variable is a named pointer to a memory location holding a
value that may change over time. RETRO provides two ways to
create a variable:

    'A var

The first, using `var`, creates a name and allocates one cell
for storage. The memory is initialized to zero.

    #10 'B var<n>

The second, `var<n>`, takes a value from the stack, and creates
a name, allocates one cell for storage, and then initializes it
to the value specified.

This is cleaner than doing:

    'A var
    #10 &A store

## Custom Structures

You can also create custom data structures by creating a name,
and allocating space yourself. For instance:

    'Test d:create
      #10 , #20 , #30 ,

This would create a data structure named `Test`, with three
values, initialized to 10, 20, and 30. The values would be
stored in consecutive memory locations.

If you want to allocate a specific amount of space for future
use, you could use `allot` here:

    'Buffer d:create
      #2048 allot

The use of `allot` reserves space, but does not initialize the
space.

Tip: If you want to deallocate space, pass a negative value to
     `allot`. Be careful though: if you have added new words or
     data after the allocation this will cause them to be
     overwritten, which can lead to crashes or bugs.

## Strings

In addition to the basic data structures above, RETRO also
provides support for string data.

Creating a string simply requires using the `'` prefix:

    'this_is_a_string
    '__this_string_has_leading_and_trailing_spaces__

When creating strings, RETRO uses a floating, rotating buffer
for temporary strings. Strings created in a definition are
permanent, but mutable.

Note the use of underscores in place of spaces. Since strings
are handled by a prefix handler, the token can not contain
whitespace characters. RETRO will convert the underscores into
spaces by default.

### Length

You can obtain the length of a string using `s:length`: 

    'this_is_a_string s:length

### Comparisons

Strings can be compared using `s:eq?`:

    'test_1  'test_2  s:eq? n:put
    >>> 0
    'test_3  'test_3  s:eq? n:put
    >>> -1

The comparisons are case sensitive.

============= To Be Continued ...

# Quotes and Combinators

RETRO leverages two concepts that need some explanation. These
are quotes and combinators.

A *quote* is simply an anonymous, nestable function. They can
be created at any time.

Example:

    #12 [ dup * ] call

In this, the code stating with `[` and ending with `]` is the
quote. Here it's just `call`ed immediately, but you can also
pass it to other words.

We use the word *combinator* to refer to words that operate on
quotes.

You'll use quotes and combinators extensively for controlling
the flow of execution. This begins with conditionals.

Assuming that we have a flag on the stack, you can run a quote
if the flag is `TRUE`:

    #1 #2 eq?
    [ 'True! s:put ] if

Or if it's `FALSE`:

    #1 #2 eq?
    [ 'Not_true! s:put ] -if

There's also a `choose` combinator:

    #1 #2 eq?
    [ 'True! s:put     ]
    [ 'Not_true! s:put ] choose

RETRO also uses combinators for loops.

A counted loop takes a count and a quote:

    #0 #100 [ dup n:put sp n:inc ] times

You can also loop while a quote returns a flag of `TRUE`:

    #0 [ n:inc dup #100 lt? ] while

Or while it returns `FALSE`:

    #100 [ n:dec dup n:zero? ] until

Combinators are also used to iterate over data structures. For
instance, many structures provide a `for-each` combinator which
can be run once for each item in the structure. E.g., with a
string:

    'Hello [ c:put ] s:for-each

Moving further, combinators are also used for filters and
operations on data. Again with strings:

    'Hello_World! [ c:-vowel? ] s:filter

This runs `s:filter` which takes a quote returning a flag. For
each `TRUE` it appends the character into a new string, while
`FALSE` results are discarded.

You might also use a `map`ping combinator to update a data set:

    'Hello_World [ c:to-upper ] s:map

This takes a quote that modifies a value which is then used to
build a new string.

There are many more combinators. Look in the Glossary to find
them. Some notable ones include:

    bi
    bi*
    bi@
    tri
    tri*
    tri@
    dip
    sip

---

# I/O

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

---

# Q & A

Q: Is RETRO Actually Useful?

A: Yes. I use it daily for handling a large number of small
   tasks. The HTTP and Gopher servers in the examples host
   both my personal sites and a half dozen other small
   websites. Most programs I write start off as prototypes
   in RETRO, even though I may need to rewrite them in other
   languages due to project requirements.
   
   On a larger scale, RETRO code is used in an order managment
   system used by an electrial wholesale company, where it gets
   used daily.
   
   It's also been used in data analysis, in an embedded
   prototype for vehicle tracking, and as part of an energy
   auditing tool.

Q: What Systems Are Supported?

A: Many. RETRO has a flexible I/O model that allows it to be
   tailored to a variety of hosts. The most full featured
   systems are BSD Unix, Linux, macOS, and iOS. It also runs
   on Windows, Haiku, FreeDOS, and directly on raw x86
   hardware. It's been built and reported to work on x86,
   x86-64, MIPS32, ARM, and PowerPC.

---

