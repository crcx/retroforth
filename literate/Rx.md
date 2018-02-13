    ____  _   _
    || \\ \\ //
    ||_//  )x(
    || \\ // \\ 2018.2
    a minimalist forth for nga

*Rx* (*retro experimental*) is a minimal Forth implementation for the
Nga virtual machine. Like Nga this is intended to be used within a
larger supporting framework adding I/O and other desired functionality.
Various example interface layers are included.

## General Notes on the Source

Rx is developed using a literate tool called *unu*. This allows easy
extraction of fenced code blocks into a separate file for later
compilation. Developing in a literate approach is beneficial as it
makes it easier for me to keep documentation current and lets me
approach the code in a more structured manner.

This source is written in Muri, an assembler for Nga.

## In the Beginning...

All code built with the Nga toolchain starts with a jump to the main
entry point. With cell packing, this takes two cells. We can take
advantage of this knowledge to place a couple of variables at the
start so they can be easily identified and interfaced with external
tools. This is important as Nga allows for a variety of I/O models to
be implemented and I don't want to tie Rx into any one specific model.

Here's the initial memory map:

| Offset | Contains                    | Notes              |
| ------ | --------------------------- | ------------------ |
| 0      | lit call nop nop            | Compiled by *Naje* |
| 1      | Pointer to main entry point | Compiled by *Naje* |
| 2      | Dictionary                  |                    |
| 3      | Heap                        |                    |

Naje, the Nga assembler, compiles the initial instructions automatically.
Muri does not, so provide this here.


~~~
i liju....
d -1
~~~

The two variables need to be declared next, so:

~~~
: Dictionary
r 9999

: Heap
d 1536

: Version
d 201802
~~~

Both of these are pointers. `Dictionary` points to the most recent
dictionary entry. (See the *Dictionary* section at the end of this
file.) `Heap` points to the next free memory address. This is hard
coded to an address beyond the end of the Rx kernel. It'll be fine
tuned as development progresses. See the *Interpreter &amp; Compiler*
section for more on this.

## Nga Instruction Set

The core Nga instruction set consists of 27 instructions. Rx begins by
assigning each to a separate function. These are not intended for direct
use; in Rx the compiler will fetch the opcode values to use from these
functions when compiling. Some of them will also be wrapped in normal
functions later.

~~~
: _nop
d 0
i re......

: _lit
d 1
i re......

: _dup
d 2
i re......

: _drop
d 3
i re......

: _swap
d 4
i re......

: _push
d 5
i re......

: _pop
d 6
i re......

: _jump
d 7
i re......

: _call
d 8
i re......

: _ccall
d 9
i re......

: _ret
d 10
i re......

: _eq
d 11
i re......

: _neq
d 12
i re......

: _lt
d 13
i re......

: _gt
d 14
i re......

: _fetch
d 15
i re......

: _store
d 16
i re......

: _add
d 17
i re......

: _sub
d 18
i re......

: _mul
d 19
i re......

: _divmod
d 20
i re......

: _and
d 21
i re......

: _or
d 22
i re......

: _xor
d 23
i re......

: _shift
d 24
i re......

: _zret
d 25
i re......

: _end
d 26
i re......
~~~

Nga also allows for multiple instructions to be packed into a single
memory location (called a *cell*). Rx doesn't take advantage of this
yet, with the exception of calls. Since calls take a value from the
stack, a typical call (in Muri assembly) would look like:

    i lica....
    r bye

Without packing this takes three cells: one for the lit, one for the
address, and one for the call. Packing drops it to two since the
lit/call combination can be fit into a single cell. We define the
opcode for this here so that the compiler can take advantage of the
space savings.

Likewise, I define a packed jump for use with quotations. This saves
several hundred cells (and thus fetch/decode cycles) when loading the
standard library.

## Memory

The basic memory accesses are handled via `fetch` and `store`. These
two functions provide slightly easier access to linear sequences of data.

`fetch-next` takes an address and fetches the stored value. It returns
the next address and the stored value.

~~~
: fetch-next
i duliadsw
d 1
i fere....
~~~

`store-next` takes a value and an address. It stores the value to the
address and returns the next address.

~~~
: store-next
i duliadpu
d 1
i stpore..
~~~


## Conditionals

The Rx kernel provides three conditional forms:

    flag true-pointer false-pointer choose
    flag true-pointer if
    flag false-pointer -if

Implement `choose`, a conditional combinator which will execute one of
two functions, depending on the state of a flag. We take advantage of
a little hack here. Store the pointers into a jump table with two
fields, and use the flag as the index. Default to the *false* entry,
since a *true* flag is -1.

~~~
: choice:true
d 0

: choice:false
d 0

: choose
i listlist
r choice:false
r choice:true
i liadfeca
r choice:false
i re......
~~~

Next the two *if* forms. Note that I allow *-if* to fall through
into *if*. This saves two cells of memory.

~~~
: -if
i pulieqpo
d 0
: if
i cc......
i re......
~~~

## Strings

The kernel needs two basic string operations for dictionary searches:
obtaining the length and comparing for equality.

Strings in Rx are zero terminated. This is a bit less elegant than
counted strings, but the implementation is quick and easy.

First up, string length. The process here is trivial:

* Make a copy of the starting point
* Fetch each character, comparing to zero

  * If zero, break the loop
  * Otherwise discard and repeat

* When done subtract the original address from the current one
* Then subtract one (to account for the zero terminator)

~~~
: count
i lica....
r fetch-next
i zr......
i drliju..
r count

: s:length
i dulica..
r count
i lisuswsu
d 1
i re......
~~~

String comparisons are harder.

  dup fetch push n:inc swap
  dup fetch push n:inc pop dup pop
  -eq? [ drop-pair drop #0 pop pop drop drop ]
       [ 0; drop s:eq? pop pop drop drop ] choose drop-pair #-1 ;

~~~
: mismatch
i drdrdrli
d 0
i popodrdr
i re......

: matched
i zr......
i drlica..
r s:eq
i popodrdr
i re......

: s:eq
i dufepuli
d 1
i adswdufe
i puliadpo
d 1
i duponeli
r mismatch
i lilica..
r matched
r choose
i drdrlire
d -1
~~~

## Interpreter & Compiler

### Compiler Core

The heart of the compiler is `comma` which stores a value into memory
and increments a variable (`Heap`) pointing to the next free address.
`here` is a helper function that returns the address stored in `Heap`.

~~~
: comma
i lifelica
r Heap
r store-next
i listre..
r Heap
~~~

With these we can add a couple of additional forms. `comma:opcode` is
used to compile VM instructions into the current defintion. This is
where those functions starting with an underscore come into play. Each
wraps a single instruction. Using this we can avoid hard coding the
opcodes.

This performs a jump to the `comma` word instead of using a `call/ret`
to save a cell and slightly improve performance.

~~~
: comma:opcode
i feliju..
r comma
~~~

`comma:string` is used to compile a string into the current definition.
As with `comma:opcode`, this uses a `jump` to eliminate the final tail
call.

~~~
: ($)
i lica....
r fetch-next
i zr......
i lica....
r comma
i liju....
r ($)

: comma:string
i lica....
r ($)
i drliliju
d 0
r comma
~~~

With the core functions above it's now possible to setup a few more
things that make compilation at runtime more practical.

First, a variable indicating whether we should compile or run a function.
This will be used by the *word classes*.

~~~
: Compiler
d 0
~~~

~~~
: t-;
i lilica..
r _ret
r comma:opcode
i lilistre
d 0
r Compiler
~~~

### Word Classes

Rx is built over the concept of *word classes*. Word classes are a way
to group related words, based on their compilation and execution
behaviors. A special word, called a *class handler*, is defined to
handle an execution token passed to it on the stack. The compiler uses
a variable named class to set the default class when compiling a word.
We'll take a closer look at class later. Rx provides several classes
with differing behaviors:

`class:data` provides for dealing with data structures.

| interpret            | compile                       |
| -------------------- | ----------------------------- |
| leave value on stack | compile value into definition |

~~~
: class:data
i lifezr..
r Compiler
i drlilica
r _lit
r comma:opcode
i liju....
r comma
~~~

`class:word` handles most functions.

| interpret            | compile                       |
| -------------------- | ----------------------------- |
| call a function      | compile a call to a function  |

~~~
: class:word:interpret
i ju......

: class:word:compile
i lilica..
d 2049 packed li/ca/../..
r comma
i liju....
r comma

: class:word
i lifelili
r Compiler
r class:word:compile
r class:word:interpret
i liju....
r choose
~~~

`class:primitive` is a special class handler for functions that
correspond to Nga instructions.

| interpret            | compile                                     |
| -------------------- | ------------------------------------------- |
| call the function    | compile the instruction into the definition |

~~~
: class:primitive
i lifelili
r Compiler
r comma:opcode
r class:word:interpret
i liju....
r choose
~~~

`class:macro` is the class handler for *compiler macros*. These are
functions that always get called. They can be used to extend the
language in interesting ways.

| interpret            | compile                       |
| -------------------- | ----------------------------- |
| call the function    | call the function             |

~~~
: class:macro
i ju......
~~~

The class mechanism is not limited to these classes. You can write
custom classes at any time. On entry the custom handler should take the
XT passed on the stack and do something with it. Generally the handler
should also check the `Compiler` state to determine what to do in either
interpretation or compilation.

### Dictionary

Rx has a single dictionary consisting of a linked list of headers. The
current form of a header is shown in the chart below. Pay special
attention to the accessors. Each of these words corresponds to a field
in the dictionary header. When dealing with dictionary headers, it is
recommended that you use the accessors to access the fields since it is
expected that the exact structure of the header will change over time.

| field | holds                                       | accessor |
| ----- | ------------------------------------------- | -------- |
| link  | link to the previous entry, 0 if last entry | d:link   |
| xt    | link to start of the function               | d:xt     |
| class | link to the class handler function          | d:class  |
| name  | zero terminated string                      | d:name   |

The initial dictionary is constructed at the end of this file. It'll
take a form like this:

    : 0000
    d 0
    r _dup
    r class:primitive
    s dup

    : 0001
    r 0000
    r _drop
    r class:primitive
    s drop

    : 0002
    r 0001
    r _swap
    r class:primitive
    s swap

Each entry starts with a pointer to the prior entry (with a pointer
to zero marking the first entry in the dictionary), a pointer to
the start of the function, a pointer to the class handler, and a nul
terminated string indicating the name exposed to the Rx interpreter.

Rx will store the pointer to the most recent entry in a variable
called `Dictionary`. For simplicity, we just assign the last entry
an arbitrary label of 9999. This is set at the start of the source.
(See *In the Beginning...*)

Rx provides accessor functions for each field. Since the number of
fields (or their ordering) may change over time, using these reduces
the number of places where field offsets are hard coded.

~~~
: d:link
i re......

: d:xt
i liadre..
d 1

: d:class
i liadre..
d 2

: d:name
i liadre..
d 3
~~~

A traditional Forth has `create` to make a new dictionary entry
pointing to the next free location in `Heap`. Rx has `newentry` which
serves as a slightly more flexible base. You provide a string for the
name, a pointer to the class handler, and a pointer to the start of
the function. Rx does the rest.

~~~
: newentry
i lifepuli
r Heap
r Dictionary
i felica..
r comma
i lica....
r comma
i lica....
r comma
i lica....
r comma:string
i polistre
r Dictionary
~~~

Rx doesn't provide a traditional create as it's designed to avoid
assuming a normal input stream and prefers to take its data from the
stack.

### Dictionary Search

~~~
: Which
d 0

: Needle
d 0

: found
i listlire
r Which
r _nop

: find
i lilistli
d 0
r Which
r Dictionary
i fe......

: find_next
i zr......
i dulica..
r d:name
i lifelica
r Needle
r s:eq
i licc....
r found
i feliju..
r find_next

: d:lookup
i listlica
r Needle
r find
i lifere..
r Which
~~~

### Number Conversion

This code converts a zero terminated string into a number. The approach
is very simple:

* Store an internal multiplier value (-1 for negative, 1 for positive)
* Clear an internal accumulator value
* Loop:

  * Fetch the accumulator value
  * Multiply by 10
  * For each character, convert to a numeric value and add to the
    accumulator
  * Store the updated accumulator

* When done, take the accumulator value and the modifier and multiply
  them to get the final result

At this time Rx only supports decimal numbers.

~~~
: next
i lica....
r fetch-next
i zr......
i lisuswpu
d 48
i swlimuad
d 10
i poliju..
r next

: check
i dufelieq
d 45
i zr......
i drswdrli
d -1
i swliadre
d 1

: s:to-number
i liswlica
d 1
r check
i liswlica
d 0
r next
i drmure..
~~~

### Token Processing

An input token has a form like:

    <prefix-char>string

Rx will check the first character to see if it matches a known prefix.
If it does, it will pass the string (without the token) to the prefix
handler. If not, it will attempt to find the token in the dictionary.

Prefixes are handled by functions with specific naming conventions. A
prefix name should be:

    prefix:<prefix-char>

Where <prefix-char> is the character for the prefix. These should be
compiler macros (using the `class:macro` class) and watch the `compiler`
state to decide how to deal with the token. To find a prefix, Rx stores
the prefix character into a string named `prefixed`. It then searches
for this string in the dictionary. If found, it sets an internal
variable (`prefix:handler`) to the dictionary entry for the handler
function. If not found, `prefix:handler` is set to zero. The check,
done by `prefix?`, also returns a flag.

~~~
: prefix:no
d 32
d 0

: prefix:handler
d 0

: prefixed
s prefix:_

: prefix:prepare
i feliliad
r prefixed
d 7
i stre....

: prefix:has-token?
i dulica..
r s:length
i lieqzr..
d 1
i drdrlire
r prefix:no

: prefix?
i lica....
r prefix:has-token?
i lica....
r prefix:prepare
i lilica..
r prefixed
r d:lookup
i dulistli
r prefix:handler
d 0
i nere....
~~~

Rx uses prefixes for important bits of functionality including parsing
numbers (prefix with `#`), obtaining pointers (prefix with `&`), and
starting new functions (using the `:` prefix).

I use `jump` for tail call eliminations here.

| prefix | used for          | example |
| ------ | ----------------- | ------- |
| #      | numbers           | #100    |
| $      | ASCII characters  | $e      |
| &      | pointers          | &swap   |
| :      | definitions       | :foo    |
| (      | Comments          | (n-)    |

~~~
: prefix:(
i drre....

: prefix:#
i lica....
r s:to-number
i liju....
r class:data

: prefix:$
i feliju..
r class:data

: prefix::
i lilifeli
r class:word
r Heap
r newentry
i ca......
i lifelife
r Heap
r Dictionary
i lica....
r d:xt
i stlilist
d -1
r Compiler
i re......

: prefix:&
i lica....
r d:lookup
i lica....
r d:xt
i feliju..
r class:data
~~~

### Quotations

Quotations are anonymous, nestable blocks of code. Rx uses them for
control structures and some aspects of data flow. A quotation takes a
form like:

    [ #1 #2 ]
    #12 [ square #144 eq? [ #123 ] [ #456 ] choose ] call

Begin a quotation with `[` and end it with `]`.

~~~
: t-[
i lifeliad
r Heap
d 2
i lifelili
r Compiler
d -1
r Compiler
i stlilica
d 1793 packed li/ju/../..
r comma
i lifelili
r Heap
d 0
r comma
i ca......
i lifere..
r Heap

: t-]
i lilica..
r _ret
r comma:opcode
i lifeswli
r Heap
r _lit
i lica....
r comma:opcode
i lica....
r comma
i swstlist
r Compiler
i lifezr..
r Compiler
i drdrre..
~~~

## Lightweight Control Structures

Rx provides a couple of functions for simple flow control apart from
using quotations. These are `repeat`, `again`, and `0;`. An example of
using them:

    :s:length dup [ repeat fetch-next 0; drop again ] call swap - #1 - ;

These can only be used within a definition or quotation. If you need
to use them interactively, wrap them in a quote and `call` it.

~~~
: repeat
i lifere..
r Heap

: again
i lilica..
r _lit
r comma:opcode
i lica....
r comma
i liliju..
r _jump
r comma:opcode

: t-0;
i liliju..
r _zret
r comma:opcode

: t-push
i liliju..
r _push
r comma:opcode

: t-pop
i liliju..
r _pop
r comma:opcode
~~~

## Interpreter

The *interpreter* is what processes input. What it does is:

* Take a string
* See if the first character has a prefix handler

  * Yes: pass the rest of the string to the prefix handler for
         processing
  * No: lookup in the dictionary

    * Found: pass xt of word to the class handler for processing
    * Not found: report error via `err:notfound`

First up, the handler for dealing with words that are not found. This
is defined here as a jump to the handler for the Nga *NOP* instruction.
It is intended that this be hooked into and changed.

As an example, in Rx code, assuming an I/O interface with some support
for strings and output:

    [ $? putc space 'word not found' puts ]
    &err:notfound #1 + store

~~~
: err:notfound
i liju....
r _nop
~~~

`call:dt` takes a dictionary token and pushes the contents of the `d:xt`
field to the stack. It then calls the class handler stored in `d:class`.

~~~
: call:dt
i dulica..
r d:xt
i feswlica
r d:class
i feju....
~~~

~~~
: input:source
d 0

: interpret:prefix
i lifezr..
r prefix:handler
i lifeliad
r input:source
d 1
i swliju..
r call:dt

: interpret:word
i lifeliju
r Which
r call:dt

: interpret:noprefix
i lifelica
r input:source
r d:lookup
i linelili
d 0
r interpret:word
r err:notfound
i liju....
r choose

: interpret
i dulistli
r input:source
r prefix?
i ca......
i lililiju
r interpret:prefix
r interpret:noprefix
r choose
~~~

## The Initial Dictionary

The dictionary is a linked list. This sets up the initial dictionary.
Maintenance of this bit is annoying, but it generally shouldn't be
necessary to change this unless you are adding new functions to the
Rx kernel. 

~~~
: 0000
d 0
r _dup
r class:primitive
s dup
: 0001
r 0000
r _drop
r class:primitive
s drop
: 0002
r 0001
r _swap
r class:primitive
s swap
: 0003
r 0002
r _call
r class:primitive
s call
: 0004
r 0003
r _eq
r class:primitive
s eq?
: 0005
r 0004
r _neq
r class:primitive
s -eq?
: 0006
r 0005
r _lt
r class:primitive
s lt?
: 0007
r 0006
r _gt
r class:primitive
s gt?
: 0008
r 0007
r _fetch
r class:primitive
s fetch
: 0009
r 0008
r _store
r class:primitive
s store
: 0010
r 0009
r _add
r class:primitive
s +
: 0011
r 0010
r _sub
r class:primitive
s -
: 0012
r 0011
r _mul
r class:primitive
s *
: 0013
r 0012
r _divmod
r class:primitive
s /mod
: 0014
r 0013
r _and
r class:primitive
s and
: 0015
r 0014
r _or
r class:primitive
s or
: 0016
r 0015
r _xor
r class:primitive
s xor
: 0017
r 0016
r _shift
r class:primitive
s shift
: 0018
r 0017
r t-push
r class:macro
s push
: 0019
r 0018
r t-pop
r class:macro
s pop
: 0020
r 0019
r t-0;
r class:macro
s 0;
: 0021
r 0020
r fetch-next
r class:word
s fetch-next
: 0022
r 0021
r store-next
r class:word
s store-next
: 0023
r 0022
r s:to-number
r class:word
s s:to-number
: 0024
r 0023
r s:eq
r class:word
s s:eq?
: 0025
r 0024
r s:length
r class:word
s s:length
: 0026
r 0025
r choose
r class:word
s choose
: 0027
r 0026
r if
r class:word
s if
: 0028
r 0027
r -if
r class:word
s -if
: 0029
r 0028
r prefix:(
r class:macro
s prefix:(
: 0030
r 0029
r Compiler
r class:data
s Compiler
: 0031
r 0030
r Heap
r class:data
s Heap
: 0032
r 0031
r comma
r class:word
s ,
: 0033
r 0032
r comma:string
r class:word
s s,
: 0034
r 0033
r t-;
r class:macro
s ;
: 0035
r 0034
r t-[
r class:macro
s [
: 0036
r 0035
r t-]
r class:macro
s ]
: 0037
r 0036
r Dictionary
r class:data
s Dictionary
: 0038
r 0037
r d:link
r class:word
s d:link
: 0039
r 0038
r d:xt
r class:word
s d:xt
: 0040
r 0039
r d:class
r class:word
s d:class
: 0041
r 0040
r d:name
r class:word
s d:name
: 0042
r 0041
r class:word
r class:word
s class:word
: 0043
r 0042
r class:macro
r class:word
s class:macro
: 0044
r 0043
r class:data
r class:word
s class:data
: 0045
r 0044
r newentry
r class:word
s d:add-header
: 0046
r 0045
r prefix:#
r class:macro
s prefix:#
: 0047
r 0046
r prefix::
r class:macro
s prefix::
: 0048
r 0047
r prefix:&
r class:macro
s prefix:&
: 0049
r 0048
r prefix:$
r class:macro
s prefix:$
: 0050
r 0049
r repeat
r class:macro
s repeat
: 0051
r 0050
r again
r class:macro
s again
: 0052
r 0051
r interpret
r class:word
s interpret
: 0053
r 0052
r d:lookup
r class:word
s d:lookup
: 0054
r 0053
r class:primitive
r class:word
s class:primitive
: 0055
r 0054
r Version
r class:data
s Version
: 9999
r 0055
r err:notfound
r class:word
s err:notfound
~~~

## Appendix: Words, Stack Effects, and Usage

| Word            | Stack     | Notes                                             |
| --------------- | --------- | ------------------------------------------------- |
| dup             | n-nn      | Duplicate the top item on the stack               |
| drop            | nx-n      | Discard the top item on the stack                 |
| swap            | nx-xn     | Switch the top two items on the stack             |
| call            | p-        | Call a function (via pointer)                     |
| eq?             | nn-f      | Compare two values for equality                   |
| -eq?            | nn-f      | Compare two values for inequality                 |
| lt?             | nn-f      | Compare two values for less than                  |
| gt?             | nn-f      | Compare two values for greater than               |
| fetch           | p-n       | Fetch a value stored at the pointer               |
| store           | np-       | Store a value into the address at pointer         |
| +               | nn-n      | Add two numbers                                   |
| -               | nn-n      | Subtract two numbers                              |
| *               | nn-n      | Multiply two numbers                              |
| /mod            | nn-mq     | Divide two numbers, return quotient and remainder |
| and             | nn-n      | Perform bitwise AND operation                     |
| or              | nn-n      | Perform bitwise OR operation                      |
| xor             | nn-n      | Perform bitwise XOR operation                     |
| shift           | nn-n      | Perform bitwise shift                             |
| fetch-next      | a-an      | Fetch a value and return next address             |
| store-next      | na-a      | Store a value to address and return next address  |
| push            | n-        | Move value from data stack to address stack       |
| pop             | -n        | Move value from address stack to data stack       |
| 0;              | n-n OR n- | Exit word (and `drop`) if TOS is zero             |
| s:to-number     | s-n       | Convert a string to a number                      |
| s:eq?           | ss-f      | Compare two strings for equality                  |
| s:length        | s-n       | Return length of string                           |
| choose          | fpp-?     | Execute *p1* if *f* is -1, or *p2* if *f* is 0    |
| if              | fp-?      | Execute *p* if flag *f* is true (-1)              |
| -if             | fp-?      | Execute *p* if flag *f* is false (0)              |
| Compiler        | -p        | Variable; holds compiler state                    |
| Heap            | -p        | Variable; points to next free memory address      |
| ,               | n-        | Compile a value into memory at `here`             |
| s,              | s-        | Compile a string into memory at `here`            |
| ;               | -         | End compilation and compile a *return* instruction|
| [               | -         | Begin a quotation                                 |
| ]               | -         | End a quotation                                   |
| Dictionary      | -p        | Variable; points to most recent header            |
| d:link          | p-p       | Given a DT, return the address of the link field  |
| d:xt            | p-p       | Given a DT, return the address of the xt field    |
| d:class         | p-p       | Given a DT, return the address of the class field |
| d:name          | p-p       | Given a DT, return the address of the name field  |
| class:word      | p-        | Class handler for standard functions              |
| class:primitive | p-        | Class handler for Nga primitives                  |
| class:macro     | p-        | Class handler for immediate functions             |
| class:data      | p-        | Class handler for data                            |
| d:add-header    | saa-      | Add an item to the dictionary                     |
| prefix:#        | s-        | # prefix for numbers                              |
| prefix::        | s-        | : prefix for definitions                          |
| prefix:&        | s-        | & prefix for pointers                             |
| prefix:$        | s-        | $ prefix for ASCII characters                     |
| repeat          | -a        | Start an unconditional loop                       |
| again           | a-        | End an unconditional loop                         |
| interpret       | s-?       | Evaluate a token                                  |
| d:lookup        | s-p       | Given a string, return the DT (or 0 if undefined) |
| err:notfound    | -         | Handler for token not found errors                |

## Legalities

Rx is Copyright (c) 2016-2018, Charles Childers

Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted, provided that the
above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

My thanks go out to Michal J Wallace, Luke Parrish, JGL, Marc Simpson,
Oleksandr Kozachuk, Jay Skeer, Greg Copeland, Aleksej Saushev, Foucist,
Erturk Kocalar, Kenneth Keating, Ashley Feniello, Peter Salvi, Christian
Kellermann, Jorge Acereda, Remy Moueza, John M Harrison, and Todd Thomas.
All of these great people helped in the development of Retro 10 & 11,
without which Rx wouldn't have been possible.
