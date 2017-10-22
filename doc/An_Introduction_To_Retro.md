========================
An Introduction to RETRO
========================

---------------
Getting Started
---------------

Building the VM
===============
RETRO runs on a virtual machine. This has been implemented in many
languages, and allows easy portability to most platforms. The primary
implementation is in C.

Building it on FreeBSD, Linux or macOS is just a matter of running:

    make

It'll build with the standard C compilers (tested with gcc and clang),
and requires a standard `make` (tested with BSD and GNU variants).

When done you will have a few files of interest:

    ngaImage    Contains the RETRO system as a memory image

    bin/repl    Interactive interface, requires ngaImage in
                the current directory

    bin/rre     Batch/Scripting interface; this is self-
                contained and embeds the ngaImage into itself
                during the build process

I symlink the `bin/rre` to a location in my $PATH so I can refer to
it from anywhere in my environment.


Basic Interactions with the REPL
================================
When you start RETRO via `bin/repl`, you should see something like
the following:

    RETRO 12 (rx-2017.11)
    8388608 MAX, TIB @ 1025, Heap @ 9374

At this point you are at the *listener*, which reads and processes
your input. You are now set to begin exploring RETRO.

To exit, run `bye`:

    bye

Basic Interactions with RRE
===========================
`rre` (short for *run retro and exit*) is my preferred interface. It
takes a filename and runs the code in the file. E.g.,

    rre example/99Bottles.forth

Source files for `rre` are written in a literate format, with code
stored in Markdown-style fenced blocks. E.g.,

    Define a word that returns the cube of a number.

    ~~~
    :n:cube (n-n)  dup dup * * ;
    ~~~

`rre` adds significant features to the base language, including
support for keyboard input, file i/o, fetching resources via
gopher, and floating point support.

----------------------
Exploring the Language
----------------------

Names And Numbers
=================
At a fundamental level, the RETRO language consists of whitespace 
delimited tokens.

The interpreter takes a look at the first character of the token. If
this matches a known *prefix*, the rest of the token is passed to a
function which handles that group of tokens. If no valid prefix is
found, RETRO tries to find the word in the dictionary. If successful
the  information in the *dictionary header* is used to carry out the 
actions specified in the name's *definition*. If this also fails,
RETRO calls `err:notfound` to report the error.

A flowchart of the interpreter process:

    +-------------+     +------------------------------------+
    | Get a Token | ==> | Is first character a valid prefix? |
    +-------------+     +------------------------------------+
                          |                    |
                         YES                  NO
                          |                   |
              +-----------------------+  +---------------------------+
              | Pass rest of token to |  | Is entire token a name in |
              | prefix handler        |  | the dictionary?           |
              +-----------------------+  +---------------------------+
                                            |               |
                                           YES             NO
                                            |              |
                            +------------------+  +-------------------+
                            | Push XT to stack |  | Call err:notfound |
                            +------------------+  +-------------------+
                                     |
                           +--------------------+
                           | Call class handler |
                           +--------------------+

In RETRO, the *prefix handlers* and *class handlers* are responsible
for dealing with the tokens and words. This includes both interpret
and compilation as necessary.

RETRO permits names to contain any characters other than space, tab,
cr, dnd lf. Names are *case sensitive*, so the following are three
*different* names from RETRO's perspective:

    foo Foo FOO

Note that a name should not start with a *prefix* as prefixes are
checked prior to dictionary lookups.

The Compiler
============
To create new functions, you use the compiler. This is generally
started by using the `:` (pronounced *colon*) prefix. A simple
example:

    :foo #1 #2 + putn ;

Breaking this apart:

    :foo

RETRO sees that `:` is a prefix and calls the handler for it. The
handler creates a new word named *foo* and starts the compiler.

    #1

RETRO sees that `#` is a prefix and calls the handler for it. The
handler converts the token to a number and then pushes the value to
the stack. Since the `Compiler` is now active, it then pops the
value off the stack and compiles it into the current definition.

    #2

And again, but compile a *2* instead of a *1*.


   +

RETRO does not find a `+` prefix, so it searches the dictionary.
Upon finding `+`, it pushes the address (*xt*) to the stack and
calls the corresponding class handler. The class handler for
normal words calls the code at the address if interpreting, or
compiles a call to it if the `Compiler` is active.

    putn

The process is repeated for `putn`.

    ;

The last word has a slight difference. Like `+` and `putn`, this
is a word, not a prefixed token. But the class handler for this
one always calls the associated code. In this case, `;` is the
word which ends a definition and turns off the `Compiler`.

Hyperstatic Global Environment
==============================
This now brings up an interesting subpoint. RETRO provides what is
sometimes called a *hyper-static global environment.* This can be
difficult to explain, so let's take a quick look at how it works:

    :scale (x-y) A fetch * ;
    >>> A ?
    #1000 'A var<n>
    :scale (x-y) A fetch * ;
    #3 scale putn
    >>> 3000
    #100 A store
    #3 scale putn
    >>> 300
    #5 'A var<n>
    #3 scale putn
    >>> 300
    A fetch putn
    >>> 5

Output is marked with **>>>**.

Note that we create two variables with the same name (*A*). The 
definition for *scale* still refers to the old variable, even
though we can no longer directly manipulate it.

In a hyper-static global environment, functions continue to refer to
the variables and functions that existed when they were defined. If
you create a new variable or function with the same name as an existing
one, it only affects future code.

Take advantage of this to reuse short names whenever it helps to
make your code easier to understand.

Classes
=======
Getting back to function creation, it's time for a clarification: in 
RETRO, the interpreter is unaware of how to handle a dictionary entry
and has no concept of the difference between compiling and interpreting.

The actual work is handled by something we call *class handlers*.

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

When a token is found, the listener pushes the contents of the xt field 
to the stack and then calls the function that the class handler field
points to.

This model differs from Forth (and most other languages) in that the 
interpreter is unaware of how tokens are handled. All actions are
performed by the class handlers, which allows for easy addition of new
categories of words and functionality.


Data Structures
===============
You can create named data structures using `d:create`, `var`, `var<n>`,
and `const`.


Constants
---------
These are the simplest data structure. The *xt* is set to a value, which 
is either left on the stack or compiled into a definition.

    #100 'ONE-HUNDRED const

By convention, constants in RETRO should have names in all uppercase.


Variables
---------
A variable is a named pointer to a memory location holding a value that 
may change over time. RETRO provides two ways to create a variable:

    'A var

The first, using `var`, creates a name and allocates one cell for  storage.
The memory is initialized to zero.

    #10 'B var<n>

The second, `var<n>`, takes a value from the stack, and creates a  name,
allocates one cell for storage, and then initializes it to the value 
specified.

This is cleaner than doing:

    'A var
    #10 &A store


Custom Structures
-----------------
You can also create custom data structures by creating a name, and 
allocating space yourself. For instance:

    'Test d:creat
      #10 , #20 , #30 ,

This would create a data structure named `Test`, with three values, 
initialized to 10, 20, and 30. The values would be stored in
consecutive memory  locations.

If you want to allocate a buffer, you could use `allot` here:

    'Buffer d:create
      #2048 allot

The use of `allot` reserves space, but does not initialize the space.


Strings
-------
In addition to the basic data structures above, RETRO also provides 
support for string data.

Creating a string simply requires using the `'` prefix:

    'this_is_a_string
    '__this_string_has_leading_and_trailing_spaces__

When creating strings, RETRO uses a floating, rotating buffer for 
temporary strings. Strings created in a definition are considered
permanent.

Note the use of underscores in place of spaces. Since strings are
handled by a prefix handler, the token can not contain whitespace
characters. RETRO will convert the underscores into spaces by
default.

You can obtain the length of a string using `s:length`: 

    'this_is_a_string s:length


Comparisons
-----------
Strings can be compared using `s:eq?`:

  'test_1  'test_2  s:eq? putn
  >>> 0
  'test_3  'test_3  s:eq? putn
  >>> -1

The comparisons are case sensitive.

============= To Be Continued ...
