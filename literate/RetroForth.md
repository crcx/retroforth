# RETRO FORTH

## Background

Retro is a dialect of Forth. It builds on the barebones Rx
core, expanding it into a flexible and useful language.

Over the years the language implementation has varied
substantially. Retro began in 1998 as a 16-bit assembly
implementation for x86 hardware, evolved into a 32-bit
system with cmForth and ColorForth influences, and
eventually started supporting mainstream OSes. Later it
was rewritten for a small, portable virtual machine.

This is the twelfth generation of Retro. It targets a virtual
machine (called Nga) and runs on a wide variety of host
systems.

### Namespaces

Various past releases have had different methods of dealing
with the dictionary. I have settled on using a] single global
dictionary, with a convention of using a short namespace prefix
for grouping related words. This was inspired by Ron Aaron's
8th language.

The main namespaces are:

    | namespace  | words related to   |
    | ---------- | ------------------ |
    | ASCII      | ASCII Constants    |
    | c          | characters         |
    | compile    | compiler functions |
    | d          | dictionary headers |
    | err        | error handlers     |
    | n          | numbers            |
    | s          | strings            |
    | set        | sets (arrays)      |
    | v          | variables          |

This makes it very easy to identify related words, especially
across namespaces. E.g.,

    c:put
    c:to-upper
    s:put
    s:to-upper

### Prefixes

Prefixes are an integral part of Retro. These are single symbol
modifiers added to the start of a word which control how Retro
processes the word.

The interpreter model is covered in *Rx.md*, but basically:

    - Get a token (whitespace delimited string)
    - Pass it to `interpret`
      + if the token starts with a known prefix then pass
        it to the prefix handler
      + if the initial character is not a known prefix,
        look it up
        - if found, push the address ("xt") to the stack
          and call the word's class handler
        - if not found call `err:not-found`
    - repeat as needed

This is different than the process in traditional Forth. A few
observations:

    - there are no parsing words
    - numbers are handled using a prefix
    - prefixes can be added or changed at any time

The basic prefixes are:

    | prefix | used for               |
    | ------ | ---------------------- |
    | :      | starting a definition  |
    | &      | obtaining pointers     |
    | (      | stack comments         |
    | `      | inlining bytecodes     |
    | '      | strings                |
    | #      | numbers                |
    | $      | characters             |
    | @      | variable get           |
    | !      | variable set           |

### Naming and Style Conventions

* Names should start with their namespace (if appropriate)
* Word names should be lowercase
* Variable names should be Title case
* Constants should be UPPERCASE
* Names may not start with a prefix character
* Names returning a flag should end with a ?
* Words with an effect on the stack should have a stack comment

## Code Begins

Memory Map

This assumes that the VM defines an image as being 524,288
cells. Nga implementations may provide varying amounts of
memory, so the specific addresses will vary.

    | RANGE           | CONTAINS                     |
    | --------------- | ---------------------------- |
    | 0 - 1024        | rx kernel                    |
    | 1025 - 1535     | token input buffer           |
    | 1536 +          | start of heap space          |
    | ............... | free memory for your use     |
    | 506879          | buffer for string evaluate   |
    | 507904          | temporary strings (32 * 512) |
    | 524287          | end of memory                |

I provide a word, `EOM`, which returns the last addressable
location. This will be used by the words in the `s:` namespace
to allocate the temporary string buffers at the end of memory.

~~~
:EOM  (-n)  #-3 fetch ;
~~~

## Stack Depth

`depth` returns the number of items on the data stack. This is
provided by the VM upon reading from address *-1*.

~~~
:depth  (-n) #-1 fetch ;
~~~

## Stack Comments

Stack comments are terse notes that indicate the stack effects
of words. While not required, it's helpful to include these.

They take a form like:

    (takes-returns)

I use a single character for each input and output item. These
will often (though perhaps not always) be:

    n, m, x, y  number
    a, p        pointer
    q           quotation (pointer)
    d           dictionary header (pointer)
    s           string
    c           character (ASCII)

## Dictionary Shortcuts

I define a few words in the `d:` namespace to make it easier
to operate on the most recent header in the dictionary. These
return the values in specific fields of the header.

~~~
:d:last        (-d) &Dictionary fetch ;
:d:last<xt>    (-a) d:last d:xt fetch ;
:d:last<class> (-a) d:last d:class fetch ;
:d:last<name>  (-s) d:last d:name ;
~~~

## Changing A Word's Class Handler

I implement `reclass` to change the class of the most recent
word.

~~~
:reclass    (a-) d:last d:class store ;
~~~

With this I can then define `immediate` (for state-smart words)
and `data` to tag data words.

~~~
:immediate  (-)  &class:macro reclass ;
:data       (-)  &class:data  reclass ;
~~~

~~~
:primitive (-) &class:primitive reclass ;
~~~

## Optimizations & Compiler Extensions

I have a `compile` namespace for some low level words that
compile specific Nga bytecode. This is intended to aid in
readability when constructing compiler extensions.

~~~
:compile:lit  (a-) #1 , , ;
:compile:jump (a-) #1793 , , ;
:compile:call (a-) #2049 , , ;
:compile:ret  (-)  #10 , ;
~~~

The compiler state is stored in a value named `Compiler`. I
have an accessor word that aids in readability.

~~~
:compiling?  (-f)  &Compiler fetch ;
~~~

It's sometimes useful to inline values directly. I use a
backtick prefix for this.

~~~
:prefix:`  (s-)
  s:to-number , ; immediate
~~~

It's traditional to have a word named `here` which returns the
next free address in memory.

~~~
:here  (-a)
  &Heap fetch ;
~~~

## Variables

The next two are additional prefixes to make working with
variables a bit less painful. By default you have to do things
like:

    &Name fetch #10 * &Name store

Or using combinators:

    &Name [ fetch #10 * ] sip store

With the @ and ! prefixes this can become:

    @Name #10 * !Name

When compiling, these will generate packed Nga instructions
corresponding to:

    lit + fetch + nop + nop    'life....   #3841
    lit + store + nop + nop    'list....   #4097

~~~
:prefix:@  (s-n)
  d:lookup d:xt fetch
  compiling? [ (life....) #3841 , , ]
             [ fetch                ] choose ; immediate

:prefix:!  (s-n)
  d:lookup d:xt fetch
  compiling? [ (list....) #4097 , , ]
             [ store                ] choose ; immediate
~~~

The next few words aren't actually useful until the `s:`
namespace is defined. With strings and the `'` prefix they
allow creation of variables and constants.

    | To create a                  | Use a form like    |
    | ---------------------------- | ------------------ |
    | Variable                     | `'Base var`        |
    | Variable, with initial value | `#10 'Base var<n>` |
    | Constant                     | `#-1 'TRUE const`  |

The lower level kernel provides `d:add-header` to make a new
header. This is a bit ugly to use as most of the time I don't
need all of the flexibility it provides. Here I add a word to
create a new header pointing to `here`. This is then used to
build other data structures without invoking the `:` compiler.

~~~
:d:create (s-)
  (s-) &class:data #0 d:add-header
  here d:last d:xt store ;
~~~

And then the others are trivial.

~~~
:var    (s-)  d:create #0 , ;
:var<n> (ns-) d:create , ;
:const  (ns-) d:create d:last d:xt store ;
~~~

The `const` word is an example of using the dictionary and word
classes to do some optimization.It creates a header, with a
class of `class:data`, then sets the execution pointer to the
desired value. Since the data class either leaves the word
pointer on the stack or compiles it as a literal into a
definition, this allows constants to exist as just a header
with no extra runtime code.

## Stack Shufflers

The core Rx language provides a few basic stack shuffling
words: `push`, `pop`, `drop`, `swap`, and `dup`. There are
quite a few more that are useful. Some of these are provided
here.

Most of these are implemented as raw bytecodes which can be
inlined when used in definitions. The high level definitions
are:

    :tuck      (xy-yxy)   dup push swap pop ;
    :over      (xy-xyx)   push dup pop swap ;
    :nip       (xy-y)     swap drop ;
    :drop-pair (nn-)      drop drop ;

And the low level forms:

~~~
:tuck      (xy-yxy)   `100926722 ; primitive
:over      (xy-xyx)   `67502597  ; primitive
:nip       (xy-y)     `772       ; primitive
:drop-pair (nn-)      `771       ; primitive
:?dup      (n-nn|n-n) `6402      ; primitive
:dup-pair  (xy-xyxy)  over over ;
~~~

## Combinators

Retro makes use of anonymous functions called *quotations* for
much of the execution flow and stack control. The words that
operate on these quotations are called *combinators*.

Combinators are a major part of using Retro. They help in
reducing the use of lower level shuffling and allow for a
greater overall consistency in the syntax. I also find them
to help in reducing visual noise.

### Combinators: Data

`dip` executes a quotation after moving a value off the stack.
The value is restored after execution completes. These are
equivilent:

    #10 #12 [ #3 - ] dip
    #10 #12 push #3 - pop

~~~
:dip  (nq-n)  swap push call pop ;
~~~

`sip` is similar to dip, but leaves a copy of the value on the
stack while the quotation is executed. These are equivilent:

    #10 [ #3 * ] sip
    #10 dup push #3 * pop

~~~
:sip  (nq-n)  push dup pop swap &call dip ;
~~~

Apply each quote to a copy of x.

~~~
:bi  (xqq-)  &sip dip call ;
~~~

Apply q1 to x and q2 to y.

~~~
:bi*  (xyqq-) &dip dip call ;
~~~

Apply q to x and y.

~~~
:bi@  (xyq-)  dup bi* ;
~~~

Apply each quote to a copy of x.

~~~
:tri  (xqqq-)  [ &sip dip sip ] dip call ;
~~~

Apply q1 to x, q2 to y, and q3 to z.

~~~
:tri*  (xyzqqq-)  [ [ swap &dip dip ] dip dip ] dip call ;
~~~

Apply q to x, y, and z.

~~~
:tri@ dup dup tri* ;
~~~

### Combinators: Control

Execute quote until quote returns a flag of 0.

~~~
:while  (q-)
  [ repeat dup dip swap 0; drop again ] call drop ;
~~~

Execute quote until quote returns a non-zero flag.

~~~
:until  (q-)
  [ repeat dup dip swap #-1 xor 0; drop again ] call drop ;
~~~

The `times` combinator runs a quote (n) times.

~~~
:times  (nq-)
  swap [ repeat 0; #1 - push &call sip pop again ] call drop ;
~~~

`case` is a conditional combinator. It's actually pretty
useful. What it does is compare a value on the stack to a
specific value. If the values are identical, it discards the
value and calls a quote before exiting the word. Otherwise
it leaves the stack alone and allows execution to continue.

Example:

    :c:vowel?
      $a [ TRUE ] case
      $e [ TRUE ] case
      $i [ TRUE ] case
      $o [ TRUE ] case
      $u [ TRUE ] case
      drop FALSE ;

~~~
:case
  [ over eq? ] dip swap
  [ nip call #-1 ] [ drop #0 ] choose 0; pop drop drop ;

:s:case
  [ over s:eq? ] dip swap
  [ nip call #-1 ] [ drop #0 ] choose 0; pop drop drop ;
~~~

## A Shortcut

~~~
:prefix:|
  d:lookup [ d:xt fetch ] [ d:class fetch ] bi
  compiling? [ [ class:data ] dip compile:call ]
             [ call ] choose ; immediate
~~~

## Conditionals

Taking a break from combinators for a bit, I turn to some words
for comparing things. First, constants for TRUE and FALSE.

Due to the way the conditional execution works, only these
values can be used. This is different than in a traditional
Forth, where non-zero values are true.

~~~
:TRUE  (-n) #-1 ;
:FALSE (-n)  #0 ;
~~~

The basic Rx kernel doesn't provide two useful forms which I'll
provide here.

~~~
:lteq?  (nn-f)  dup-pair eq? [ lt? ] dip or ;
:gteq?  (nn-f)  dup-pair eq? [ gt? ] dip or ;
~~~

And then some numeric comparators.

~~~
:n:MAX        (-n)    #2147483647 ;
:n:MIN        (-n)    #-2147483648 ;
:n:zero?      (n-f)   #0 eq? ;
:n:-zero?     (n-f)   #0 -eq? ;
:n:negative?  (n-f)   #0 lt? ;
:n:positive?  (n-f)   #-1 gt? ;
:n:strictly-positive?  (n-f)  #0 gt? ;
:n:even?      (n-f)  #2 /mod drop n:zero? ;
:n:odd?       (n-f)  #2 /mod drop n:-zero? ;
~~~

## More Stack Shufflers.

`rot` rotates the top three values.

High level:

    :rot  (abc-bca)   [ swap ] dip swap ;

And low level, for inlining:

~~~
:rot (abc-bca) `67503109 ; primitive
~~~

## Numeric Operations

The core Rx language provides addition, subtraction,
multiplication, and a combined division/remainder. Retro
expands on this.

I implement the division and remainder as low level words
so they can be inlined. Here's the high level forms:

    :/         (nq-d)  /mod nip ;
    :mod       (nq-r)  /mod drop ;

~~~
:/         (nq-d)  `197652 ; primitive
:mod       (nq-r)  `788    ; primitive
:not       (n-n)   #-1 xor ;
:n:pow     (bp-n)  #1 swap [ over * ] times nip ;
:n:negate  (n-n)   #-1 * ;
:n:square  (n-n)   dup * ;
:n:sqrt    (n-n)
  #1 [ repeat dup-pair / over - #2 / 0; + again ] call nip ;
:n:min     (nn-n)  dup-pair lt? [ drop ] [ nip ] choose ;
:n:max     (nn-n)  dup-pair gt? [ drop ] [ nip ] choose ;
:n:abs     (n-n)   dup n:negate n:max ;
:n:limit   (nlu-n) swap push n:min pop n:max ;
:n:inc     (n-n)   #1 + ;
:n:dec     (n-n)   #1 - ;
:n:between? (nul-) rot [ rot rot n:limit ] sip eq? ;
~~~

Some of the above, like `n:inc`, are useful with variables. But
it's messy to execute sequences like:

  @foo n:inc !foo

The `v:` namespace provides words which simplify the overall
handling of variables. With this, the above can become simply:

  &foo v:inc

~~~
:v:inc-by  (na-)   [ fetch + ] sip store ;
:v:dec-by  (na-)   [ fetch swap - ] sip store ;
:v:inc     (n-n)   #1 swap v:inc-by ;
:v:dec     (n-n)   #1 swap v:dec-by ;
:v:limit   (alu-)
  push push dup fetch pop pop n:limit swap store ;
:v:on      (a-)    TRUE swap store ;
:v:off     (a-)    FALSE swap store ;
:allot     (n-)    &Heap v:inc-by ;
~~~

`v:preserve` is a combinator that executes a quotation while
preserving the contents of a variable.

E.g., instead of:

    @Base [ #16 !Base ... ] dip !Base

You can do:

    &Base [ #16 !Base ... ] v:preserve

This is primarily to aid in readability. I find it to be
helpful when revisiting older code as it makes the intent
a bit clearer.

~~~
:v:preserve (aq-)
  swap dup fetch [ [ call ] dip ] dip swap store ;
~~~

If you need to update a stored variable there are two typical
forms:

    #1 'Next var<n>
    @Next #10 * !Next

Or:

    #1 'Next var<n>
    &Next [ fetch #10 * ] sip store

`v:update-using` replaces this with:

    #1 'Next var<n>
    &Next [ #10 * ] v:update-using

It takes care of preserving the variable address, fetching the
stored value, and updating with the resulting value.

~~~
:v:update-using (aq-) swap [ fetch swap call ] sip store ;
~~~

I have a simple word `copy` which copies memory to another
location.

~~~
:copy   (aan-) [ &fetch-next dip store-next ] times drop drop ;
~~~

## Lexical Scope

Now for something tricky: a system for lexical scoping.

The dictionary is a simple linked list. Retro allows for some
control over what is visible using the `{{`, `---reveal---`,
and `}}` words.

As an example:

    {{
      :increment dup fetch n:inc swap store ;
      :Value `0 ; data
    ---reveal---
      :next-number @Value &Value increment ;
    }}

Only the `next-number` function will remain visible once `}}`
is executed.

It's important to note that this only provides a *lexical*
scope. Any variables are *global* (though the names may be
hidden), so use `v:preserve` if you need reentrancy.

~~~
:ScopeList `0 `0 ;
:{{            (-)
  d:last dup &ScopeList store-next store ;
:---reveal---  (-)
   d:last &ScopeList n:inc store ;
:}}            (-)
  &ScopeList fetch-next swap fetch eq?
  [ @ScopeList !Dictionary ]
  [ @ScopeList
    [ &Dictionary repeat
        fetch dup fetch &ScopeList n:inc fetch -eq? 0; drop
      again ] call store ] choose ;
~~~

## Linear Buffers

A buffer is a linear memory buffer. Retro provides a `buffer:`
namespace for working with them.

This is something I've used for years. It's simple, but makes
it easy to construct strings (as it writes a trailing ASCII
null) and other simple structures.

    | word            | used for                               |
    | --------------- | -------------------------------------- |
    | buffer:start    | return the first address in the buffer |
    | buffer:end      | return the last address in the buffer  |
    | buffer:add      | add a value to the end of the buffer   |
    | buffer:get      | remove & return the last value         |
    | buffer:empty    | remove all values from the buffer      |
    | buffer:size     | return the number of stored values     |
    | buffer:set      | set an address as the start of the     |
    |                 | buffer                                 |
    | buffer:preserve | preserve the current buffer pointers & |
    |                 | execute a quotation that may set a new |
    |                 | buffer. restores the saved pointers    |
    |                 | when done                              |
    
~~~
{{
  :Buffer `0 ; data
  :Ptr    `0 ; data
  :terminate (-) #0 @Ptr store ;
---reveal---
  :buffer:start  (-a) @Buffer ;
  :buffer:end    (-a) @Ptr ;
  :buffer:add    (c-) buffer:end store &Ptr v:inc terminate ;
  :buffer:get    (-c) &Ptr v:dec buffer:end fetch terminate ;
  :buffer:empty  (-)  buffer:start !Ptr terminate ;
  :buffer:size   (-n) buffer:end buffer:start - ;
  :buffer:set    (a-) !Buffer buffer:empty ;
  :buffer:preserve (q-)
    @Buffer @Ptr [ [ call ] dip !Buffer ] dip !Ptr ;
}}
~~~

## Strings

Traditional Forth systems have a messy mix of strings. You have
counted strings, address/length pairs, and sometimes other
forms.

Retro uses zero terminated strings. Counted strings are better
in many ways, but I've used these for years and they are a
workable approach. (Though caution in needed to avoid buffer
overflow).

Temporary strings are allocated in a circular pool (`STRINGS`).
This space can be altered as needed by adjusting these
variables.

~~~
:TempStrings ;   data  #32 !TempStrings
:TempStringMax ; data #512 !TempStringMax
:STRINGS   EOM @TempStrings @TempStringMax * - ;

{{
  :Current `0 ; data

  :s:pointer (-p)  @Current @TempStringMax * STRINGS + ;
  :s:next    (-)
    &Current v:inc
    @Current @TempStrings eq? [ #0 !Current ] if ;
---reveal---
  :s:temp (s-s) dup s:length n:inc s:pointer swap copy
                s:pointer s:next ;
  :s:empty (-s) s:pointer s:next #0 over store ;
}}
~~~

Permanent strings are compiled into memory. To skip over them a
helper function is used. When compiled into a definition this
will look like:

    i lica....
    r s:skip
    d 98
    d 99
    d 100
    d 0

It'd be faster to compile a jump over the string instead. I use
this approach as it makes it simpler to identify strings when
debugging.

`s:skip` is the helper function which adjusts the Nga instruction
pointer to skip to the code following the stored string.

~~~
:s:skip (-)
  pop [ fetch-next n:-zero? ] while n:dec push ;

:s:keep (s-s)
  compiling? [ &s:skip compile:call ] if
  here [ s, ] dip class:data ;
~~~

And now a quick `'` prefix. (This will be replaced later). What
this does is either move the string token to the temporary
buffer or compile it into the current definition.

This doesn't support spaces. I use underscores instead. E.g.,

    'Hello_World!

Later in the code I'll add a better implementation which can
handle conversion of _ into spaces.

~~~
:prefix:' compiling? [ s:keep ] [ s:temp ] choose ; immediate
~~~

`s:chop` removes the last character from a string.

~~~
:s:chop (s-s) s:temp dup s:length over + n:dec #0 swap store ;
~~~

`s:reverse` reverses the order of a string. E.g.,

    'hello  ->  'olleh

~~~
:s:reverse (s-s)
  [ dup s:temp buffer:set &s:length [ dup s:length + n:dec ] bi swap
    [ dup fetch buffer:add n:dec ] times drop buffer:start s:temp ]
  buffer:preserve ;
~~~

Trimming removes leading (`s:trim-left`) or trailing
(`s:trim-right`) spaces from a string. `s:trim` removes
both leading and trailing spaces.

~~~
:s:trim-left (s-s)
  s:temp [ fetch-next [ #32 eq? ] [ n:-zero? ] bi and ] while
  n:dec ;
:s:trim-right (s-s) s:temp s:reverse s:trim-left s:reverse ;
:s:trim (s-s) s:trim-right s:trim-left ;
~~~

`s:prepend` and `s:append` for concatenating strings together.

~~~
:s:prepend (ss-s)
  s:temp [ dup s:length + [ dup s:length n:inc ] dip swap copy ] sip ;
:s:append (ss-s) swap s:prepend ;
~~~

`s:for-each` executes a quote once for each cell in string. It is
a key part of building the other high-level string operations.

~~~
:s:for-each (sq-)
  [ repeat
      over fetch 0; drop
      dup-pair
      [ [ &fetch dip call ] dip ] dip
      [ n:inc ] dip
    again
  ] call drop-pair ;
~~~

Building on `s:for-each`, I am able to implement `s:index-of`, which
finds the first instance of a character in a string.

~~~
:s:index-of (sc-n)
  swap [ repeat
           fetch-next 0; swap
           [ over -eq? ] dip
           swap 0; drop
         again
       ] sip
  [ - n:dec nip ] sip
  s:length over eq? [ drop #-1 ] if ;
~~~

`s:contains-char?` returns a flag indicating whether or not a
given character is in a string.

~~~
:s:contains-char? (sc-f) s:index-of #-1 -eq? ;
~~~

`s:contains-string?` returns a flag indicating whether or not
a given substring is in a string.

~~~
{{
  'Src var
  'Tar var
  'Pad var
  'I   var
  'F   var
  'At  var

  :terminate (-)
    #0 @Pad @Tar s:length + store ;

  :extract (-)
    @Src @I + @Pad @Tar s:length copy ;

  :compare (-)
    @Pad @Tar s:eq? @F or !F @F [ @I !At ] -if ;

  :next (-)
    &I v:inc ;
---reveal---
  :s:contains-string? (ss-f)
    !Tar !Src s:empty !Pad #0 !I #0 !F
    @Src s:length
    [ extract terminate compare next ] times
    @F ;

  :s:index-of-string (ss-a)
    !Tar !Src s:empty !Pad #0 !I #0 !F #-1 !At
    @Src s:length
    [ extract terminate compare next ] times
    @F [ @At ] [ #-1 ] choose ;
}}
~~~

`s:filter` returns a new string, consisting of the characters
from another string that are filtered by a quotation.

    'This_is_a_test [ c:-vowel? ] s:filter

~~~
:s:filter (sq-s)
  [ s:empty buffer:set swap
    [ dup-pair swap call
        [ buffer:add ]
        [ drop       ] choose
    ] s:for-each drop buffer:start
  ] buffer:preserve ;
~~~

`s:map` Return a new string resulting from applying a quotation
to each character in a source string.

    'This_is_a_test [ $_ [ ASCII:SPACE ] case ] s:map

~~~
:s:map (sq-s)
  [ s:empty buffer:set swap
    [ over call buffer:add ]
    s:for-each drop buffer:start
  ] buffer:preserve ;
~~~

`s:substr` returns a subset of a string. Provide it with a
string, a starting offset, and a length.

~~~
:s:substr (sfl-s)
  [ + s:empty ] dip [ over [ copy ] dip ] sip
  over [ + #0 swap store ] dip ;
~~~

`s:right` and `s:left` are similar to `s:substr`, but operate
from fixed ends of the string.

~~~
:s:right (sn-s) over s:length over - swap s:substr ;
:s:left  (sn-s) #0 swap s:substr ;
~~~

Hash (using DJB2)

I use the djb2 hash algorithm for computing hashes from
strings. There are better hashes out there, but this is
pretty simple and works well for my needs. This was based
on an implementation at http://www.cse.yorku.ca/~oz/hash.html

~~~
:s:hash (s-n) #5381 swap [ swap #33 * + ] s:for-each ;
~~~

Copy a string, including the terminator.

~~~
:s:copy (ss-) over s:length n:inc copy ;
~~~

RETRO provides string constants for several ranges of
characters that are of some general interest.

~~~
:s:DIGITS          (-s)  '0123456789 ;
:s:ASCII-LOWERCASE (-s)  'abcdefghijklmnopqrstuvwxyz ;
:s:ASCII-UPPERCASE (-s)  'ABCDEFGHIJKLMNOPQRSTUVWXYZ ;
:s:ASCII-LETTERS   (-s)
  'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ;
:s:PUNCTUATION     (-s)
  '_!"#$%&'()*+,-./:;<=>?@[\]^`{|}~ $_ over store ;
's:WHITESPACE d:create
  #32, #9 , #10 , #13 , #0 ,
~~~

## ASCII Constants

Not all characters can be obtained via the $ prefix. ASCII has
many characters that aren't really intended to be printable.
Retro has an `ASCII` namespace providing symbolic names for
these.

~~~
#0  'ASCII:NUL const    #1   'ASCII:SOH const
#2  'ASCII:STX const    #3   'ASCII:ETX const
#4  'ASCII:EOT const    #5   'ASCII:ENQ const
#6  'ASCII:ACK const    #7   'ASCII:BEL const
#8  'ASCII:BS  const    #9   'ASCII:HT  const
#10 'ASCII:LF  const    #11  'ASCII:VT  const
#12 'ASCII:FF  const    #13  'ASCII:CR  const
#14 'ASCII:SO  const    #15  'ASCII:SI  const
#16 'ASCII:DLE const    #17  'ASCII:DC1 const
#18 'ASCII:DC2 const    #19  'ASCII:DC3 const
#20 'ASCII:DC4 const    #21  'ASCII:NAK const
#22 'ASCII:SYN const    #23  'ASCII:ETB const
#24 'ASCII:CAN const    #25  'ASCII:EM  const
#26 'ASCII:SUB const    #27  'ASCII:ESC const
#28 'ASCII:FS  const    #29  'ASCII:GS  const
#30 'ASCII:RS  const    #31  'ASCII:US  const
#32 'ASCII:SPACE const  #127 'ASCII:DEL const
~~~

These words operate on character values. Retro currently deals
with ASCII, though cells are 32 bits in length, so Unicode
values can be stored.

First are a bunch of words to help identify character values.

~~~
:c:letter?      (c-f) $A $z n:between? ;
:c:lowercase?   (c-f) $a $z n:between? ;
:c:uppercase?   (c-f) $A $Z n:between? ;
:c:digit?       (c-f) $0 $9 n:between? ;
:c:visible?     (c-f) #31 #126 n:between? ;
:c:vowel?       (c-f) 'aeiouAEIOU swap s:contains-char? ;
:c:consonant?   (c-f)
  dup c:letter? [ c:vowel? not ] [ drop FALSE ] choose ;

{{
  'WS d:create
    ASCII:SPACE , ASCII:HT , ASCII:LF , ASCII:CR , #0 ,
---reveal---
  :c:whitespace?  (c-f) &WS swap s:contains-char? ;
}}
~~~

And the inverse forms. (These are included for readability and
orthiginal completion).

~~~
:c:-lowercase?  (c-f)  c:lowercase?  not ;
:c:-uppercase?  (c-f)  c:uppercase?  not ;
:c:-digit?      (c-f)  c:digit?      not ;
:c:-whitespace? (c-f)  c:whitespace? not ;
:c:-visible?    (c-f)  c:visible?    not ;
:c:-vowel?      (c-f)  c:vowel?      not ;
:c:-consonant?  (c-f)  c:consonant?  not ;
~~~

The next few words perform simple transformations.

~~~
:c:to-upper     (c-c) dup c:lowercase? 0; drop ASCII:SPACE - ;
:c:to-lower     (c-c) dup c:uppercase? 0; drop ASCII:SPACE + ;
:c:to-string    (c-s) '. s:temp [ store ] sip ;
:c:toggle-case  (c-c)
  dup c:lowercase? [ c:to-upper ] [ c:to-lower ] choose ;
:c:to-number    (c-n)
  dup c:digit? [ $0 - ] [ drop #0 ] choose ;
~~~

## Back to Strings

With the character transformations a few more string words are
possible.

~~~
:s:to-upper  (s-s)  [ c:to-upper ] s:map ;
:s:to-lower  (s-s)  [ c:to-lower ] s:map ;
~~~

Convert a decimal (base 10) number to a string.

~~~
{{
  'Value var
  :correct (c-c)
    dup $0 lt? [ $0 over - #2 * + ] if ; 
---reveal---
  :n:to-string  (n-s)
    [ here buffer:set dup !Value n:abs
      [ #10 /mod swap $0 + correct buffer:add dup n:-zero? ] while drop
      @Value n:negative? [ $- buffer:add ] if
      buffer:start s:reverse s:temp ] buffer:preserve ;
}}
~~~

Now replace the old prefix:' with this one that can optionally
turn underscores into spaces.

~~~
TRUE 'RewriteUnderscores var<n>

{{
  :sub (c-c) $_ [ ASCII:SPACE ] case ;
  :rewrite (s-s)
    @RewriteUnderscores [ &sub s:map ] if &prefix:' call ;
---reveal---
  :prefix:' rewrite ; immediate
}}
~~~

The `s:split` splits a string on the first instance of a given
character. Results are undefined if the character can not be
located.

~~~
:s:split (sc-ss)
  dup-pair s:index-of nip dup-pair s:left [ + ] dip ;

:s:split-on-string (ss-ss)
  dup-pair s:index-of-string n:inc nip dup-pair s:left [ + ] dip ;

:s:replace (sss-s)
  over s:length here store
  [ s:split-on-string swap here fetch + ] dip s:prepend s:append ;
~~~

`s:tokenize` takes a string and a character to use as a
separator. It splits the string into a set of substrings and
returns a set containing pointers to each of them.

~~~
{{
  'Split-On var
  :match?    (c-f) @Split-On eq? ;
  :terminate (s-s) #0 over n:dec store ;
  :step      (ss-s)
    [ n:inc ] dip match? [ dup , terminate ] if ;
---reveal---
  :s:tokenize (sc-a)
    !Split-On s:keep
    here #0 , [ dup , dup [ step ] s:for-each drop ] dip
    here over - n:dec over store ;
}}
~~~

`s:tokenize-on-string` is like `s:tokenize`, but for strings.

~~~
{{
  'Tokens var
  'Needle var
  :-match?    (s-sf)
    dup @Needle s:contains-string? ;
  :save-token (s-s)
    @Needle s:split-on-string s:keep buffer:add n:inc ;
  :tokens-to-set (-a)
    here @Tokens buffer:size dup , [ fetch-next , ] times drop ;
---reveal---
  :s:tokenize-on-string (ss-a)
    [ s:keep !Needle here #8192 + !Tokens
      @Tokens buffer:set
      [ repeat -match? 0; drop save-token again ] call
      s:keep buffer:add tokens-to-set ] buffer:preserve ;
}}
~~~

Use `s:format` to construct a string from multiple items. This
can be illustrated with:

    #4 #6 #10  '%n-%n=%n\n  s:format

The format language is simple:

    | \r | Replace with a CR                         |
    | \n | Replace with a LF                         |
    | \t | Replace with a TAB                        |
    | \\ | Replace with a single \                   |
    | \  | Replace with an underscore (_)            |
    | \0 | Replace with NUL                          |
    | %c | Replace with a character on the stack     |
    | %s | Replace with a string on the stack        |
    | %n | Replace with the next number on the stack |

~~~
{{
  :char (c-)
    ASCII:SPACE [ $_ buffer:add ] case
    $r [ ASCII:CR  buffer:add ] case
    $n [ ASCII:LF  buffer:add ] case
    $t [ ASCII:HT  buffer:add ] case
    $0 [ ASCII:NUL buffer:add ] case
    buffer:add ;

  :string (a-a)
    repeat fetch-next 0; buffer:add again ;

  :type (aac-)
    $c [ swap buffer:add              ] case
    $s [ swap string drop             ] case
    $n [ swap n:to-string string drop ] case
    drop ;

  :handle (ac-a)
    $\ [ fetch-next char ] case
    $% [ fetch-next type ] case
    buffer:add ;
---reveal---
  :s:format (...s-s)
    [ s:empty [ buffer:set
      [ repeat fetch-next 0; handle again ]
      call drop ] sip ] buffer:preserve ;
}}
~~~

~~~
:s:const (ss-)  [ s:keep ] dip const ;
~~~

## The Ultimate Stack Shuffler

Ok, This is a bit of a hack, but very useful at times.

Assume you have a bunch of values:

    #3 #1 #2 #5

And you want to reorder them into something new:

    #1 #3 #5 #5 #2 #1

Rather than using a lot of shufflers, `reorder` simplfies this
into:

    #3 #1 #2 #5
    'abcd  'baddcb reorder

~~~
{{
  'Values var #27 allot
  :from s:length dup [ [ &Values + store ] sip n:dec ] times drop ;
  :to dup s:length [ fetch-next $a -  n:inc &Values + fetch swap ] times drop ;
---reveal---
  :reorder (...ss-?) [ from ] dip to ;
}}
~~~

## Extending The Language

`does` is intended to be paired with `d:create` to attach an
action to a newly created data structure. An example use might
be something like:

    :constant (ns-)  d:create , [ fetch ] does ;

In a traditional Forth this is similar in spirit to DOES>.

~~~
:curry (vp-p)
  here [ swap compile:lit compile:call compile:ret ] dip ;
:does  (q-)
  d:last<xt> swap curry d:last d:xt store &class:word reclass ;
~~~

`d:for-each` is a combinator which runs a quote once for each
header in the dictionary. A pointer to each header will be
passed to the quote as it is run.

This can be used for implementing `words`:

    [ d:name s:put sp ] d:for-each

Or finding the length of the longest name in the dictionary:

    #0 [ d:name s:length n:max ] d:for-each

It's a handy combinator that lets me quickly walk though the
entire dictionary in a very clean manner.

~~~
:d:for-each (q-)
  &Dictionary [ repeat fetch 0;
 dup-pair [ [ swap call ] dip ] dip again ] call drop ;
~~~

Using `d:for-each`, I implement a means of looking up a
dictionary header by the `d:xt` field.

~~~
:d:lookup-xt (a-d)
 #0 swap [ dup-pair d:xt fetch eq?
           [ swap &nip dip ] &drop choose ] d:for-each drop ;
~~~

## Sets

Sets are statically sized arrays. They are represented in
memory as:

    count
    data #1 (first)
    ...
    data #n (last)

Since the count comes first, a simple `fetch` will suffice to
get it, but for completeness (and to allow for future changes),
we wrap this as `set:length`:

~~~
:set:length (a-n) fetch ;
~~~

~~~
:set:counted-results (q-a)
  call here [ dup , &, times ] dip ;
~~~

The first couple of words are used to create sets. The first,
`set:from-results` executes a quote and constructs a set from
the returned values.

~~~
:set:from-results (q-a)
  depth [ call ] dip depth swap -
  here [ dup , [ , ] times ] dip ;
~~~

The second, `set:from-string`, creates a new string with the
characters in given a string.

~~~
:set:from-string (s-a)
  here [ dup s:length , [ , ] s:for-each ] dip ;
~~~

A very crucial piece is `set:for-each`. This runs a quote once
against each value in a set. This is leveraged to implement
additional combinators.

~~~
{{
  'Q var
---reveal---
  :set:for-each (aq-)
    &Q [ !Q fetch-next
         [ fetch-next swap [ @Q call ] dip ] times drop
       ] v:preserve ;
}}
~~~

With this I can easily define `set:dup` to make a copy of a
set.

~~~
:set:dup (a-a)
  here [ dup fetch , [ , ] set:for-each ] dip ;
~~~

Next is `set:filter`, which is extracts matching values from
a set. This is used like:

    { #1 #2 #3 #4 #5 #6 #7 #8 }
    [ n:even? ] set:filter

It returns a new set with the values that the quote returned
a `TRUE` flag for.

~~~
:set:filter (aq-)
  [ over [ call ] dip swap [ , ] [ drop ] choose ] curry
  here [ over fetch , set:for-each ] dip
  here over - n:dec over store ;
~~~

Next are `set:contains?` and `set:contains-string?` which
compare a given value to each item in the array and returns
a flag.

~~~
{{
  'F var
---reveal---
  :set:contains? (na-f)
    &F v:off
    [ over eq? @F or !F ] set:for-each
    drop @F ;

  :set:contains-string? (na-f)
    &F v:off
    [ over s:eq? @F or !F ] set:for-each
    drop @F ;
}}
~~~

I implemented `set:map` to apply a quotation to each item in a
set and construct a new set from the returned values.

Example:

    { #1 #2 #3 }
    [ #10 * ] set:map

~~~
:set:map (aq-a)
  [ call , ] curry
  here [ over fetch , set:for-each ] dip ;
~~~

You can use `set:reverse` to make a copy of a set with the
values reversed. This can be useful after a `set:from-results`.

~~~
:set:reverse (a-a)
  here [ fetch-next [ + n:dec ] sip dup ,
         [ dup fetch , n:dec ] times drop
       ] dip ;
~~~

`set:nth` provides a quick means of adjusting a set and offset
into an address for use with `fetch` and `store`.

~~~
:set:nth (an-a)
  + n:inc ;
~~~

`set:reduce` takes a set, a starting value, and a quote. It
executes the quote once for each item in the set, passing the
item and the value to the quote. The quote should consume both
and return a new value.

~~~
:set:reduce (pnp-n)
  [ swap ] dip set:for-each ;
~~~

When making a set, I often want the values in the original
order. The `set:from-results set:reverse` is a bit long, so
I'm defining a new `set:make` which wraps these.

~~~
:set:make (q-a)
  set:counted-results set:reverse ;

:{ (-)  |[ |depth |[ ; immediate
:} (-a) |] |dip |depth |swap |- |n:dec |] |set:make ; immediate
~~~

## Muri: an assembler

Muri is my minimalist assembler for Nga. This is an attempt to
implement something similar in Retro.

This requires some knowledge of the Nga architecture to be
useful. The major elements are:

**Instruction Set**

Nga has 27 instructions. These are:

    0  nop        7  jump      14  gt        21  and
    1  lit <v>    8  call      15  fetch     22  or
    2  dup        9  ccall     16  store     23  xor
    3  drop      10  return    17  add       24  shift
    4  swap      11  eq        18  sub       25  zret
    5  push      12  neq       19  mul       26  end
    6  pop       13  lt        20  divmod

The mnemonics allow for each name to be reduced to just two
characters. In the same order as above:

    no  ju  gt  an
    li  ca  fe  or
    du  cc  st  xo
    dr  re  ad  sh
    sw  eq  su  zr
    pu  ne  mu  en
    po  lt  di

Up to four instructions can be packed into a single memory
location. (You can only use *no*p after a *ju*mp, *ca*ll,
*cc*all, *re*t, or *zr*et as these alter the instruction
pointer.)

So a bundled sequence like:

    lit 100
    lit 200
    add
    ret

Would look like:

    'liliadre i
    100       d
    200       d

And:

    lit s:eq?
    call

Would become:

    'lica.... i
    's:eq?    r

Note the use of `..` instead of `no` for the nop's; this is
done to improve readability a little.

Instruction bundles are specified as strings, and are converted
to actual instructions by the `i` word. As in the standard Muri
assembler, the RETRO version uses `d` for decimal values and `r`
for references to named functions.

This is kept in the global namespace, but several portions are
kept private.

~~~
{{
~~~

I allocate a small buffer for each portion of an instruction
bundle.

~~~
  'I0 d:create #3 allot
  'I1 d:create #3 allot
  'I2 d:create #3 allot
  'I3 d:create #3 allot
~~~

The `opcode` word maps a two character instruction to an opcode
number.

~~~
  :opcode (s-n)
    '.. [ #0  ] s:case  'li [ #1  ] s:case
    'du [ #2  ] s:case  'dr [ #3  ] s:case
    'sw [ #4  ] s:case  'pu [ #5  ] s:case
    'po [ #6  ] s:case  'ju [ #7  ] s:case
    'ca [ #8  ] s:case  'cc [ #9  ] s:case
    're [ #10 ] s:case  'eq [ #11 ] s:case
    'ne [ #12 ] s:case  'lt [ #13 ] s:case
    'gt [ #14 ] s:case  'fe [ #15 ] s:case
    'st [ #16 ] s:case  'ad [ #17 ] s:case
    'su [ #18 ] s:case  'mu [ #19 ] s:case
    'di [ #20 ] s:case  'an [ #21 ] s:case
    'or [ #22 ] s:case  'xo [ #23 ] s:case
    'sh [ #24 ] s:case  'zr [ #25 ] s:case
    'en [ #26 ] s:case  'ie [ #27 ] s:case
    'iq [ #28 ] s:case  'ii [ #29 ] s:case
    drop #0 ;
~~~

I use `pack` to combine the individual parts of the instruction
bundle into a single cell.

~~~
  :pack (-n)
    &I0 opcode  &I1 opcode
    &I2 opcode  &I3 opcode
    #-24 shift   swap
    #-16 shift + swap
    #-8  shift + swap + ;
~~~

Switch to the public portion of the code.

~~~
---reveal---
~~~

With this it's pretty easy to implement the instruction bundle
handler. Named `i`, this takes a string with four instruction
names, splits it into each part, calls `opcode` on each and
then `pack` to combine them before using `,` to write them into
the `Heap`.

~~~
  :i (s-)
    dup &I0 #2 copy #2 +
    dup &I1 #2 copy #2 +
    dup &I2 #2 copy #2 +
        &I3 #2 copy
    pack , ;
~~~

The `d` word inlines a data item.

~~~
  :d (n-)  , ;
~~~

And `r` inlines a reference (pointer).

~~~
  :r (s-)  d:lookup d:xt fetch , ;
~~~

The final bits are `as{` and `}as`, which start and stop the
assembler. (Basically, they just turn the `Compiler` on and
off, restoring its state as needed).

~~~
  :as{ (-f)
    @Compiler &Compiler v:off ; immediate

  :}as (f-?)
    !Compiler ; immediate
~~~

This finishes by sealing off the private words.

~~~
}}
~~~

## Evaluating Source

The standard interfaces have their own approaches to getting
and dealing with user input. Sometimes though it'd be nicer to
have a way of evaluating code from within RETRO itself. This
provides an `evaluate` word.

~~~
{{
~~~

First, create a buffer for the string to be evaluated. This is
sized to allow for a standard FORTH block to fit, or to easily
fit a traditional 1024 character block. It's also long enough
for most source lines I expect to encounter when working with
files.

I allocate this immediately prior to the temporary string
buffers.

~~~
  :current-line (-a) STRINGS #1025 - ;
~~~

To make use of this, we need to know how many tokens are in the
input string. `count-tokens` takes care of this by filtering
out anything other than spaces and getting the size of the
remaining string.

~~~
  :count-tokens (s-n)
    [ ASCII:SPACE eq? ] s:filter s:length ;
~~~

The `next-token` word splits the remainimg string on SPACE and
returns both parts.

~~~
  :next-token (s-ss)
    ASCII:SPACE s:split ;
~~~

And then the `process-tokens` uses `next-token` and `interpret`
to go through the string, processing each token in turn. Using
the standard `interpret` word allows for proper handling of the
prefixes and classes so everything works just as if entered
directly.

~~~
  :process-tokens (sn-)
    [ next-token swap
      [ dup s:length n:-zero?
            &interpret &drop choose ] dip n:inc
    ] times interpret ;
~~~

~~~
---reveal---
~~~

And finally, tie it all together into the single exposed word
`evaluate`.

~~~
  :s:evaluate (s-...)
    current-line s:copy
    current-line dup count-tokens process-tokens ;
~~~

~~~
}}
~~~


## Loops, continued

Sometimes it's useful to be able to access a loop index. The
next word, `times<with-index>` adds this to RETRO. It also
provides `I`, `J`, and `K` words to access the index of the
current, and up to two outer loops as well.

~~~
{{
  'LP var
  'Index d:create #128 allot
  :next (-) @LP &Index + v:inc ;
  :prep (-) &LP v:inc #0 @LP &Index + store ;
  :done (-) &LP v:dec ;
---reveal---
  :I (-n) @LP &Index + fetch ;
  :J (-n) @LP &Index + n:dec fetch ;
  :K (-n) @LP &Index + #2 - fetch ;
  :times<with-index>
    prep swap
      [ repeat 0; n:dec push &call sip pop next again ] call
    drop done ;
}}
~~~

## I/O

~~~
:io:enumerate (-n)   as{ 'ie...... i }as ;
:io:query     (n-mN) as{ 'iq...... i }as ;
:io:invoke    (n-)   as{ 'ii...... i }as ;

{{
  'Slot var
---reveal---
  :io:scan-for  (n-m)
    #-1 !Slot
    io:enumerate [ I io:query nip over eq?
                   [ I !Slot ] if ] times<with-index>
    drop @Slot ;
}}
~~~

A Retro system is only required to provide a single I/O word to
the user: a word to push a single character to the output log.
This is always mapped to device 0, and is exposed as `c:putc`.

~~~
:c:put (c-) #0 io:invoke ;
~~~

This can be used to implement words that push other items to
the log.

~~~
:nl   (-)  ASCII:LF c:put ;
:sp   (-)  ASCII:SPACE c:put ;
:tab  (-)  ASCII:HT c:put ;
:s:put (s-) [ c:put ] s:for-each ;
:n:put (n-) n:to-string s:put ;
~~~

An interface layer may provide additional I/O words, but these
will not be implemented here as they are not part of the core
language.

## Debugging Aids

I provide a few debugging aids in the core language. The
examples provide much better tools, and interface layers can
provide much more than I can do here.

~~~
:d:words    (-)  [ d:name s:put sp ] d:for-each ;
:reset      (...-) depth repeat 0; push drop pop #1 - again ;
:dump-stack (-)  depth 0; drop push dump-stack pop dup n:put sp ;
~~~

From Kiyoshi Yoneda, this is a variant of `d:words` which displays
words containing a specific substring. It's useful to see words in
a specific namespace, e.g., by doing `'s: d:words-with`, or words
that likely display something: `':puts d:words-with`.

~~~
{{
  :display-if-matched  (s-)
    dup here s:contains-string? [ s:put sp ] [ drop ] choose ;
---reveal---
  :d:words-with (s-)
    here s:copy [ d:name display-if-matched ] d:for-each ;
}}
~~~

~~~
:FREE (-n) STRINGS #1025 - here - ;
~~~

## The End

## Legalities

Permission to use, copy, modify, and/or distribute this
software for any purpose with or without fee is hereby
granted, provided that the copyright notice and this
permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS
ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
OF THIS SOFTWARE.

    Copyright (c) 2008 - 2019, Charles Childers
    Copyright (c) 2012 - 2013, Michal J Wallace
    Copyright (c) 2009 - 2011, Luke Parrish
    Copyright (c) 2009 - 2010, JGL
    Copyright (c) 2010 - 2011, Marc Simpson
    Copyright (c) 2011 - 2012, Oleksandr Kozachuk
    Copyright (c) 2018,        Kiyoshi Yoneda
    Copyright (c) 2010,        Jay Skeer
    Copyright (c) 2010,        Greg Copeland
    Copyright (c) 2011,        Aleksej Saushev
    Copyright (c) 2011,        Foucist
    Copyright (c) 2011,        Erturk Kocalar
    Copyright (c) 2011,        Kenneth Keating
    Copyright (c) 2011,        Ashley Feniello
    Copyright (c) 2011,        Peter Salvi
    Copyright (c) 2011,        Christian Kellermann
    Copyright (c) 2011,        Jorge Acereda
    Copyright (c) 2011,        Remy Moueza
    Copyright (c) 2012,        John M Harrison
    Copyright (c) 2012,        Todd Thomas
