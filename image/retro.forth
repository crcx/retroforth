# RETRO FORTH

## Background

RETRO is a dialect of Forth. It builds on the barebones Rx
core, expanding it into a flexible and useful language.

Over the years the language implementation has varied
substantially. RETRO began in 1998 as a 16-bit assembly
implementation for x86 hardware, evolved into a 32-bit
system with cmForth and ColorForth influences, and
eventually started supporting mainstream OSes. Later it
was rewritten for a small, portable virtual machine.

This is the twelfth generation of RETRO. It targets a virtual
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
    | a          | arrays             |
    | buffer     | LIFO buffer        |
    | c          | characters         |
    | class      | class handlers     |
    | compile    | compiler functions |
    | d          | dictionary headers |
    | err        | error handlers     |
    | io         | i/o functions      |
    | n          | numbers            |
    | s          | strings            |
    ! sigil      | sigil handlers     |
    | v          | variables          |

This makes it very easy to identify related words, especially
across namespaces. E.g.,

    c:put
    c:to-upper
    s:put
    s:to-upper

### Sigils

Sigils are an integral part of RETRO. These are single symbol
modifiers added to the start of a word which control how RETRO
processes the word.

The interpreter model is covered in *retro.muri*, but basically:

    - Get a token (whitespace delimited string)
    - Pass it to `interpret`
      + if the token starts with a known sigil then pass
        it to the sigil handler
      + if the initial character is not a known sigil,
        look it up
        - if found, push the address ("xt") to the stack
          and call the word's class handler
        - if not found call `err:not-found`
    - repeat as needed

This is different than the process in traditional Forth. A few
observations:

    - there are no parsing words
    - numbers are handled using a sigil
    - sigils can be added or changed at any time

The basic sigils are:

    | sigil  | used for               |
    | ------ | ---------------------- |
    | :      | starting a definition  |
    | &      | obtaining pointers     |
    | (      | stack comments         |
    | `      | inlining bytecodes     |
    | '      | strings                |
    | #      | numbers                |
    | $      | characters             |
    | @      | get variable contents  |
    | !      | set variable contents  |
    | \      | inline assembly        |
    | ^      | assembly references    |
    | |      | compiler macros        |

### Naming and Style Conventions

* Names be consistent across namespaces
* Word names should be lowercase
* Variable names should be Title case
* Constants should be UPPERCASE
* Names may not start with a sigil character
* Names returning a flag should end with a ?
* Words with an effect on the stack should have a stack comment

## Code Begins

Memory Map

This assumes that the VM defines an image as being 524,288
cells. Nga implementations may provide varying amounts of
memory, so the specific addresses will vary.

    | RANGE           | CONTAINS                     |
    | --------------- | ---------------------------- |
    | 0 - 1024        | RETRO Core kernel            |
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
:d:last.xt     (-a) d:last d:xt fetch ;
:d:last.class  (-a) d:last d:class fetch ;
:d:last.name   (-s) d:last d:name ;
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
:primitive  (-)  &class:primitive reclass ;
~~~

## Hooks

In RETRO 11, nearly all definitions could be temporarily
replaced by leaving space at the start for compiling in a
jump. In the current RETRO I do not do this, though the
technique is still useful, especially with I/O. These next
few words provide a means of doing this in RETRO 12.

To allow a word to be overridden, add a call to `hook` as
the first word in the definition. This will compile a jump
to the actual definition start.

~~~
:hook (-)  #1793 , &Heap fetch #1 + , ; immediate
~~~

`set-hook` takes a pointer to the new word or quote and a
pointer to the hook to replace. It alters the jump target.

~~~
:set-hook (aa-) #1 + store ;
~~~

The final word, `unhook`, resets the jump target to the
original one.

~~~
:unhook (a-) #1 + dup #1 + swap store ;
~~~

## Visual Grouping

Comments start with a `(` and end at the first whitespace. It's
useful to be able to punctuate a code block, providing some
inline commentary and having a clear end point. We can provide
this with a single word `)`.

Example:

    :pre.min (a-an)
      (comparison  &lt? &lt-or-gt? set-hook )
      (begin_with  #-1 !Index n:min !Value dup a:length ) ;

~~~
:( ; immediate
:) ; immediate
~~~

## Optimizations & Compiler Extensions

I have a `compile` namespace for some low level words that
compile specific Nga bytecode. This is intended to aid in
readability when constructing compiler extensions.

~~~
:compile:lit  (a-) (li...... #1    , , ) ;
:compile:jump (a-) (liju.... #1793 , , ) ;
:compile:call (a-) (lica.... #2049 , , ) ;
:compile:ret  (-)  (re...... #10   ,   ) ;
~~~

The compiler state is stored in a value named `Compiler`. I
have an accessor word that aids in readability.

~~~
:compiling?  (-f)  &Compiler fetch ;
~~~

It's sometimes useful to inline values directly. I use a
backtick sigil for this.

~~~
:sigil:`  (s-)  s:to-number , ; immediate
:sigil:\  (s-)  i ; immediate
:sigil:^  (s-)  r ; immediate
~~~

It's traditional to have a word named `here` which returns the
next free address in memory.

~~~
:here  (-a)  &Heap fetch ;
~~~

## Variables

The next two are additional sigils to make working with
variables a bit less painful. By default you have to do
things like:

    &Name fetch #10 * &Name store

Or using combinators:

    &Name [ fetch #10 * ] sip store

With the @ and ! sigils this can become:

    @Name #10 * !Name

When compiling, these will generate packed Nga instructions
corresponding to:

    lit + fetch + nop + nop    'life....   #3841
    lit + store + nop + nop    'list....   #4097

~~~
:sigil:@  (s-n)
  d:lookup d:xt fetch
  compiling? [ (life.... #3841 , , ) ]
             [ fetch                 ] choose ; immediate

:sigil:!  (s-n)
  d:lookup d:xt fetch
  compiling? [ (list.... #4097 , , ) ]
             [ store                 ] choose ; immediate
~~~

The next few words aren't actually useful until the `s:`
namespace is defined. With strings and the `'` sigil they
allow creation of variables and constants.

    | To create a                  | Use a form like    |
    | ---------------------------- | ------------------ |
    | Variable                     | `'Base var`        |
    | Variable, with initial value | `#10 'Base var-n`  |
    | Constant                     | `#-1 'TRUE const`  |

The lower level kernel provides `d:add-header` to make a new
header. This is a bit ugly to use as most of the time I don't
need all of the flexibility it provides. Here I add a word to
create a new header pointing to `here`. This is then used to
build other data structures without invoking the `:` compiler.

~~~
:d:create (s-)
  hook &class:data #0 d:add-header
       here d:last d:xt store ;
~~~

And then the others are trivial.

~~~
:var-n  (ns-) d:create , ;
:var    (s-)  \liswlica `0 ^var-n ;
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

The core RETRO language provides a few basic stack shuffling
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
:tuck      (xy-yxy)   \dupuswpo ; primitive
:over      (xy-xyx)   \puduposw ; primitive
:nip       (xy-y)     \swdr.... ; primitive
:drop-pair (nn-)      \drdr.... ; primitive
:?dup      (n-nn|n-n) \duzr.... ; primitive
:dup-pair  (xy-xyxy)  over over ;
~~~

## Combinators

RETRO makes use of anonymous functions called *quotations* for
much of the execution flow and stack control. The words that
operate on these quotations are called *combinators*.

Combinators are a major part of using RETRO. They help in
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
:dip  (nq-n) \swpuca.. pop ;
~~~

`sip` is similar to dip, but leaves a copy of the value on the
stack while the quotation is executed. These are equivilent:

    #10 [ #3 * ] sip
    #10 dup push #3 * pop

~~~
:sip  (nq-n)  \puduposw &call dip ;
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
:tri* (xyzqqq-)  [ [ swap &dip dip ] dip dip ] dip call ;
~~~

Apply q to x, y, and z.

~~~
:tri@ dup dup tri* ;
~~~

### Combinators: Control

Execute quote until quote returns a flag of 0. In high level code:

  :while  (q(-f)-)
    [ repeat dup push call pop swap 0; drop again ] call drop ;

This is manually translated to assembly and inlined for performance.

~~~
:while  (q(-f)-) [ repeat \dupuca.. \poswzr.. drop again ] call drop ;
~~~

Execute quote until quote returns a non-zero flag. As with `while`
the high level code:

  :until  (q-)
    [ repeat dup dip swap not 0; drop again ] call drop ;

is manually translated to assembly and inlined for performance.

~~~
:until (q-) [ repeat \dupuca.. \poswlixo `-1 0; drop again ] call drop ;
~~~

`forever` provides an endless loop.

~~~
:forever (q-) repeat [ call ] sip again ;
~~~

The `times` combinator runs a quote (n) times.

This is defined using inlined Nga machine code. It corresponds
to:

    repeat 0; #1 - push dup push call pop pop again

~~~
:times  (nq-)
  [ swap repeat 0; \lisupudu `1 \puca.... \popo.... again ] call drop ;
~~~

## A Shortcut

~~~
:sigil:|
  d:lookup [ d:xt fetch ] [ d:class fetch ] bi
  compiling? [ &class:data dip compile:call ]
             &call choose ; immediate
~~~

## Conditionals

Taking a break from combinators for a bit, I turn to some words
for comparing things. First, constants for TRUE and FALSE.

I define `TRUE` as -1 and `FALSE` as zero, but any non-zero
value will be treated as TRUE by the conditional words.

~~~
:TRUE  (-n) #-1 ;
:FALSE (-n)  #0 ;
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
  [ nip call TRUE ] [ drop FALSE ] choose 0; pop drop drop ;

:s:case
  [ over s:eq? ] dip swap
  [ nip call TRUE ] [ drop FALSE ] choose 0; pop drop drop ;
~~~

Some numeric comparisons.

~~~
:not          (n-n)  #-1 xor ;
:lteq?        (nn-f) dup-pair \eqpultpo \or...... ;
:gteq?        (nn-f) swap lteq? ;
:n:MAX        (-n)   #-5 fetch ;
:n:MIN        (-n)   #-4 fetch ;
:n:zero?      (n-f)  #0 eq? ;
:n:-zero?     (n-f)  #0 -eq? ;
:n:negative?  (n-f)  #0 lt? ;
:n:positive?  (n-f)  #-1 gt? ;
:n:strictly-positive?  (n-f)  #0 gt? ;
:n:even?      (n-f)  #2 /mod drop n:zero? ;
:n:odd?       (n-f)  n:even? not ;
~~~

The basic RETRO kernel doesn't provide a few useful forms
which I'll provide here.

~~~
:if;   (qf-)  over &if  dip     0; pop drop-pair ;
:-if;  (qf-)  over &-if dip not 0; pop drop-pair ;
~~~

## More Stack Shufflers.

`rot` rotates the top three values.

High level:

    :rot  (abc-bca)   [ swap ] dip swap ;

And low level, for inlining:

~~~
:rot (abc-bca) \puswposw ; primitive
~~~

## Numeric Operations

The core RETRO language provides addition, subtraction,
multiplication, and a combined division/remainder. RETRO
expands on this.

I implement the division and remainder as low level words
so they can be inlined. Here's the high level forms:

    :/         (nq-d)  /mod nip ;
    :mod       (nq-r)  /mod drop ;

~~~
:/         (nq-d)  \diswdr.. ; primitive
:mod       (nq-r)  \didr.... ; primitive
:n:pow     (bp-n)  #1 swap [ over * ] times nip ;
:n:negate  (n-n)   #-1 * ;
:n:square  (n-n)   \dumu.... ;
:n:sqrt    (n-n)
  #1 [ repeat dup-pair / over - #2 / 0; + again ] call nip ;
:n:min     (nn-n)  dup-pair lt? [ drop ] [ nip ] choose ;
:n:max     (nn-n)  dup-pair gt? [ drop ] [ nip ] choose ;
:n:abs     (n-n)   dup n:negative? &n:negate if ;
:n:limit   (nlu-n) swap push n:min pop n:max ;
:n:inc     (n-n)   \liadre.. `1 ;
:n:dec     (n-n)   \lisure.. `1 ;
:n:between? (nul-) rot [ rot rot n:limit ] sip eq? ;
~~~

## Lexical Scope

Now for something tricky: a system for lexical scoping.

The dictionary is a simple linked list. RETRO allows for some
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
        \fedufe.. &ScopeList n:inc \fenezr.. drop
      again ] call store ] choose ;
~~~

## Byte Addressing

~~~
{{
  :Byte ;

  :byte-mask  (xn-b)
    #255 swap #8 * dup
    [ n:negate shift and ] dip shift ;

  :replace
    #0 [ [ [ [ drop @Byte ] dip ] dip ] dip ] case
    #1 [ [ [ drop @Byte ] dip ] dip ] case
    #2 [ [ drop @Byte ] dip ] case
    #3 [ drop @Byte ] case
    drop ;

---reveal---

  :b:to-byte-address (a-a) \limu.... `4 ;

  :b:fetch (a-b)
    \lidisw.. `4
    \lidilica `4 ^rot
    \adfesw..
    byte-mask ;

  :b:store (ba-)
    \swlist.. ^Byte
    \lidisw.. `4
    [ \dufelica ^unpack ] dip
    replace pack swap store ;
}}
~~~

Fetch/store for halfword values.

~~~
:h:fetch (a-n)
  &b:fetch [ n:inc b:fetch #-8 shift ] bi or ;
:h:store (na-)
  dup-pair \pulianpo `255 b:store
  n:inc \pulishli `8 `255 \anpoliju ^b:store ;
~~~

Fetch/store for word values (using byte level address). Note that
addresses must be aligned on a four byte boundary.

~~~
:w:fetch (a-n) #4 / fetch ;
:w:store (na-) #4 / store ;
~~~

~~~
:w:fetch-next (a-an) dup #4 + swap w:fetch ;
:h:fetch-next (a-an) dup #2 + swap h:fetch ;
:b:fetch-next (a-an) dup #1 + swap b:fetch ;

:w:store-next (na-a) dup #4 + &w:store dip ;
:h:store-next (na-a) dup #2 + &h:store dip ;
:b:store-next (na-a) dup #1 + &b:store dip ;
~~~


## Variable Operations

Some of the above, like `n:inc`, are useful with variables. But
it's messy to execute sequences like:

  @foo n:inc !foo

The `v:` namespace provides words which simplify the overall
handling of variables. With this, the above can become simply:

  &foo v:inc

~~~
:v:inc-by  (na-)   \dupufead \postre.. ;
:v:dec-by  (na-)   \dupufesw \supostre ;
:v:inc     (n-n)   #1 swap v:inc-by ;
:v:dec     (n-n)   #1 swap v:dec-by ;
:v:limit   (alu-)  \pupudufe \popo.... n:limit \swst.... ;
:v:on      (a-)    TRUE  \swst.... ;
:v:off     (a-)    FALSE \swst.... ;
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
:v:preserve (aq-)n
  \swdufepu &call dip \poswst.. ;
~~~

If you need to update a stored variable there are two typical
forms:

    #1 'Next var-n
    @Next #10 * !Next

Or:

    #1 'Next var-n
    &Next [ fetch #10 * ] sip store

`v:update` replaces this with:

    #1 'Next var-n
    &Next [ #10 * ] v:update

It takes care of preserving the variable address, fetching the
stored value, and updating with the resulting value.

~~~
:v:update (aq-) swap [ fetch swap call ] sip store ;
~~~

I have a simple word `copy` which copies memory to another
location. This originally was defined as:

    :copy   (aan-)
      [ [ fetch-next ] dip store-next ] times drop drop ;

It is now written with the loop body using inline assembly
to improve performance.

~~~
:copy  (aan-)
  [ \puduliad `1 \swfepodu \liadpust `1 \po...... ] times
  drop-pair ;
~~~

## Linear Buffers

A buffer is a linear memory buffer. RETRO provides a `buffer:`
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
  :start `0 ; data
  :end   `0 ; data
  :terminate (-) #0 @end store ;
---reveal---
  :buffer:start  (-a) @start ;
  :buffer:end    (-a) @end ;
  :buffer:add    (c-) @end store &end v:inc terminate ;
  :buffer:get    (-c) &end v:dec @end fetch terminate ;
  :buffer:empty  (-)  @start !end terminate ;
  :buffer:size   (-n) @end @start - ;
  :buffer:set    (a-) !start buffer:empty ;
  :buffer:preserve (q-) @start @end [ &call dip !start ] dip !end ;
}}
~~~

## Strings

Traditional Forth systems have a messy mix of strings. You have
counted strings, address/length pairs, and sometimes other
forms.

RETRO uses zero terminated strings. Counted strings are better
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

:s:oversize? (s-f)
  s:length @TempStringMax n:dec gt? ;

:s:truncate (s-s)
  dup s:oversize? [ #0 over @TempStringMax + store ] if ;

{{
  :Current `0 ; data

  :s:pointer (-p)  @Current @TempStringMax * STRINGS + ;
  :s:next    (-)
    &Current v:inc
    @Current @TempStrings eq? [ #0 !Current ] if ;
---reveal---
  :s:temp (s-s) s:truncate
                dup s:length n:inc s:pointer swap copy
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
  pop [ \duliadsw `1 \feline.. `0 ] while n:dec push ;

:s:keep (s-s)
  compiling? [ &s:skip compile:call ] if
  here &s, dip class:data ;
~~~

And now a quick `'` sigil. (This will be replaced later). What
this does is either move the string token to the temporary
buffer or compile it into the current definition.

This doesn't support spaces. I use underscores instead. E.g.,

    'Hello_World!

Later in the code I'll add a better implementation which can
handle conversion of _ into spaces.

~~~
:sigil:' compiling? &s:keep &s:temp choose ; immediate
~~~

`s:fetch` and `s:store` retrieve and update characters in
a string.

~~~
:s:fetch (sn-c) + fetch ;
:s:store (csn-) + store ;
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

`s:prepend` and `s:append` for concatenating strings together.

~~~
:s:prepend (ss-s)
  s:temp [ dup-pair &s:length bi@ + @TempStringMax gt?
           &drop-pair
           [ dup s:length + [ dup s:length n:inc ] dip swap copy ]
           choose ] sip ;
:s:append (ss-s) swap s:prepend ;
~~~

`s:for-each` executes a quote once for each cell in string. It is
a key part of building the other high-level string operations.

    :s:for-each (sq-)
      [ repeat
          over fetch 0; drop
          dup-pair
          [ [ &fetch dip call ] dip ] dip
          &n:inc dip
        again
      ] call drop-pair ;

~~~
:s:for-each (sq-)
  [ repeat
      \puduposw \fezr.... drop
      \puduposw \puduposw
      \pupupufe \poca.... \poliadpo `1
    again
  ] call drop-pair ;
~~~

`s:index/char` finds the first instance of a character in
a string.

~~~
:s:index/char (sc-n)
  swap
  [ repeat
      fetch-next 0; swap
      [ over -eq? ] dip swap 0; drop
    again ]
  [ - n:dec nip ]
  [ s:length over eq? ] tri
  [ drop #-1 ] if ;
~~~

`s:contains/char?` returns a flag indicating whether or not a
given character is in a string.

~~~
:s:contains/char? (sc-f) s:index/char #-1 -eq? ;
~~~

Hash (using DJB2)

I use the djb2 hash algorithm for computing hashes from
strings. There are better hashes out there, but this is
pretty simple and works well for my needs. This was based
on an implementation at http://www.cse.yorku.ca/~oz/hash.html

~~~
:s:hash (s-n) #5381 swap [ \swlimuad `33 ] s:for-each ;
~~~

`s:contains/string?` returns a flag indicating whether or not
a given substring is in a string.

~~~
{{
  'Str var
  :extract  dup-pair @Str swap copy @Str over + #0 swap store ;
  :check    &extract dip [ &n:inc dip ] dip @Str s:hash over eq? ;
  :location rot rot [ [ swap [ over n:zero? and ] dip swap [ nip dup ] if ] dip ] dip ;
  :setup    s:empty !Str #0 rot rot &s:length &s:hash bi
            [ over s:length ] dip swap ;
---reveal---
  :s:index/string (ss-n)
    over [ setup [ check location ] times
         drop-pair drop ] dip - #2 - #-1 n:max ;
}}

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
  :s:contains/string? (ss-f)
    !Tar !Src s:empty !Pad #0 !I #0 !F
    @Src s:length
    [ extract terminate compare next ] times
    @F ;
}}
~~~

`s:filter` returns a new string, consisting of the characters
from another string that are filtered by a quotation.

    'This_is_a_test [ c:-vowel? ] s:filter

~~~
:s:filter (sq-s)
  [ s:empty buffer:set swap
    [ dup-pair swap call
        &buffer:add &drop choose
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
  [ + s:empty ] dip [ over &copy dip ] sip
  over [ + #0 swap store ] dip ;
~~~

`s:right` and `s:left` are similar to `s:substr`, but operate
from fixed ends of the string.

~~~
:s:right (sn-s) over s:length over - swap s:substr ;
:s:left  (sn-s) #0 swap s:substr ;
~~~

`s:begins-with?` and `s:ends-with?` are useful words to see if
a string starts or ends with a specific substring.

~~~
:s:begins-with? (ss-f)
  dup s:length &swap dip s:left s:eq? ;

:s:ends-with? (ss-f)
  dup s:length &swap dip s:right s:eq? ;
~~~

Copy a string, including the terminator.

~~~
:s:copy (ss-) over s:length n:inc copy ;
~~~

RETRO provides string constants for several ranges of
characters that are of some general interest.

~~~
:s:DIGITS          (-s)  '0123456789ABCDEF ;
:s:ASCII-LOWERCASE (-s)  'abcdefghijklmnopqrstuvwxyz ;
:s:ASCII-UPPERCASE (-s)  'ABCDEFGHIJKLMNOPQRSTUVWXYZ ;
:s:ASCII-LETTERS   (-s)
  'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ;
:s:PUNCTUATION     (-s)
  '_!"#$%&'()*+,-./:;<=>?@[\]^`{|}~ $_ over store ;
's:WHITESPACE d:create
  #32 , #9 , #10 , #13 , #0 ,
~~~

I have a few words that correspond to empty versions of the
sigils.

~~~
:' |s:empty ; immediate
:$ #0 class:data ; immediate
~~~

## ASCII Constants

Not all characters can be obtained via the $ sigil. ASCII has
many characters that aren't really intended to be printable.
RETRO has an `ASCII` namespace providing symbolic names for
these.

The first group of these is fairly common, the others are
much less so.

~~~
#0  'ASCII:NUL const    #27  'ASCII:ESC const
#8  'ASCII:BS  const    #9   'ASCII:HT  const
#10 'ASCII:LF  const    #11  'ASCII:VT  const
#12 'ASCII:FF  const    #13  'ASCII:CR  const
#32 'ASCII:SPACE const  #127 'ASCII:DEL const

#1  'ASCII:SOH const
#2  'ASCII:STX const    #3   'ASCII:ETX const
#4  'ASCII:EOT const    #5   'ASCII:ENQ const
#6  'ASCII:ACK const    #7   'ASCII:BEL const
#14 'ASCII:SO  const    #15  'ASCII:SI  const
#16 'ASCII:DLE const    #17  'ASCII:DC1 const
#18 'ASCII:DC2 const    #19  'ASCII:DC3 const
#20 'ASCII:DC4 const    #21  'ASCII:NAK const
#22 'ASCII:SYN const    #23  'ASCII:ETB const
#24 'ASCII:CAN const    #25  'ASCII:EM  const
#26 'ASCII:SUB const
#28 'ASCII:FS  const    #29  'ASCII:GS  const
#30 'ASCII:RS  const    #31  'ASCII:US  const
~~~

These words operate on character values. RETRO currently deals
with ASCII, though cells are 32 bits in length, so Unicode
values can be stored.

First are a bunch of words to help identify character values.

~~~
:c:lowercase?   (c-f) $a $z n:between? ;
:c:uppercase?   (c-f) $A $Z n:between? ;
:c:letter?      (c-f) &c:lowercase? &c:uppercase? bi or ;
:c:digit?       (c-f) $0 $9 n:between? ;
:c:visible?     (c-f) #32 #126 n:between? ;
:c:vowel?       (c-f) 'aeiouAEIOU swap s:contains/char? ;
:c:consonant?   (c-f) dup c:letter? [ c:vowel? not ] [ drop FALSE ] choose ;
:c:whitespace?  (c-f) s:WHITESPACE swap s:contains/char? ;
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
:c:to-string    (c-s) '. s:temp &store sip ;
:c:toggle-case  (c-c) dup c:lowercase? &c:to-upper &c:to-lower choose ;
:c:to-number    (c-n) dup c:digit? [ $0 - ] [ drop #0 ] choose ;
~~~

## Back to Strings

With the character transformations a few more string words are
possible.

~~~
:s:to-upper  (s-s)  &c:to-upper s:map ;
:s:to-lower  (s-s)  &c:to-lower s:map ;
~~~

Trimming removes leading (`s:trim-left`) or trailing
(`s:trim-right`) spaces from a string. `s:trim` removes
both leading and trailing spaces.

~~~
:s:trim-left (s-s)
  s:temp [ fetch-next &c:whitespace? &n:-zero? bi and ] while n:dec ;
:s:trim-right (s-s) s:temp s:reverse s:trim-left s:reverse ;
:s:trim (s-s) s:trim-right s:trim-left ;
~~~

Now replace the old sigil:' with this one that can optionally
turn underscores into spaces.

~~~
TRUE 'RewriteUnderscores var-n

{{
  :sub     (c-c) $_ [ ASCII:SPACE ] case ;
  :rewrite (s-s) @RewriteUnderscores [ &sub s:map ] if ;
  :handle        &sigil:' call ;
---reveal---
  :sigil:' rewrite handle ; immediate
}}
~~~

The `s:split` splits a string on the first instance of a given
character. Results are undefined if the character can not be
located.

~~~
:s:split/char (sc-ss)
  dup-pair s:index/char nip dup-pair s:left &+ dip ;

:s:split/string (ss-ss)
  dup-pair s:index/string n:inc nip dup-pair s:left &+ dip ;

:s:replace (sss-s)
  over s:length here store
  [ s:split/string swap here fetch + ] dip s:prepend s:append ;
~~~

`s:tokenize` takes a string and a character to use as a
separator. It splits the string into a set of substrings and
returns an array containing pointers to each of them.

~~~
{{
  'Split-On var
  :match?    (c-f) @Split-On eq? ;
  :terminate (s-s) #0 over n:dec store ;
  :step      (ss-s)
    &n:inc dip match? [ dup , terminate ] if ;
---reveal---
  :s:tokenize (sc-a)
    !Split-On s:keep
    here #0 , [ dup , dup &step s:for-each drop ] dip
    here over - n:dec over store ;
}}
~~~

`s:tokenize-on-string` is like `s:tokenize`, but for strings.

~~~
{{
  'Needle d:create #128 allot
  'Len var
  'Tokens d:create #128 allot
  'TP var
  :save s:keep @TP &Tokens + n:inc store &TP v:inc ;
  :next [ @Len + ] sip ;
  :done? s:length n:zero? ;
---reveal---
  :s:tokenize-on-string (ss-s)
    #0 !TP
    [ dup &Needle s:copy s:append ] [ s:length !Len ] bi
    [ &Needle s:split/string save next done? ] until
    &Tokens @TP n:dec !Tokens nip ;
}}
~~~

~~~
{{
  'String d:create   #66 allot
  :check-sign (n-)   n:negative? [ $- buffer:add ] if ;
  :n->digit   (n-c)  s:DIGITS + fetch ;
  :convert    (n-)   [ @Base /mod swap n->digit buffer:add dup n:zero? ] until drop ;
---reveal---
  :n:to-string/reversed (n-s)
    [ &String buffer:set dup n:abs convert check-sign ] buffer:preserve
    &String ;
  :n:to-string (n-s)
    n:to-string/reversed s:reverse ;
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
    | \^ | Replace with ESC                          |
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
    $^ [ ASCII:ESC buffer:add ] case
    buffer:add ;

  :type (aac-)
    $c [ swap buffer:add              ] case
    $s [ swap &buffer:add s:for-each  ] case
    $n [ swap n:to-string &buffer:add s:for-each ] case
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
:s:const (ss-)  &s:keep dip const ;
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
  :from s:length dup [ [ &Values \adst.... ] sip n:dec ] times drop ;
  :to dup s:length [ fetch-next $a -  n:inc &Values \adfesw.. ] times drop ;
---reveal---
  :reorder (...ss-?) &from dip to ;
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
  here [ swap compile:lit compile:jump ] dip ;
:does  (q-)
  d:last.xt swap curry d:last d:xt store &class:word reclass ;
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
  &Dictionary [ repeat \fezr....
  dup-pair \pupuswca \popo.... again ] call drop ;
~~~

Using `d:for-each`, I implement a means of looking up a
dictionary header by the `d:xt` field.

~~~
:d:lookup-xt (a-d)
 #0 swap [ dup-pair d:xt \feeq....
           [ swap &nip dip ] &drop choose ] d:for-each drop ;
~~~

~~~
:gc (q-) &Heap swap v:preserve ;
~~~

## Arrays

RETRO provides words for statically sized arrays. They are
represented in memory as:

    count
    data #1 (first)
    ...
    data #n (last)

Since the count comes first, a simple `fetch` will suffice to
get it, but for completeness (and to allow for future changes),
we wrap this as `a:length`:

~~~
:a:length (a-n) fetch ;
~~~

To extract portions of an array, I provide `a:left`, `a:right`,
and `a:middle`.

~~~
:a:middle (afl-a)
  here [ dup , [ n:inc + ] dip
         here swap copy ] dip ;

:a:left  (an-a) #0 swap a:middle ;
:a:right (an-a) over a:length over - swap a:middle ;
~~~


The first couple of words are used to create arrays. The first,
`a:counted-results` executes a quote which returns values
and a count. It then creates an array with the provided data.

~~~
:a:counted-results (q-a)
  call here [ dup , &, times ] dip ;
~~~

The second, `a:from-string`, creates a new string with the
characters in given a string.

~~~
:a:from-string (s-a)
  here [ dup s:length , &, s:for-each ] dip ;
~~~

A very crucial piece is `a:for-each`. This runs a quote once
against each value in an array. This is leveraged to implement
additional combinators.

~~~
:a:for-each (aq-)
  swap fetch-next &swap dip
  [ push fetch-next \swpodupu \swpuca.. \popo.... ] times drop-pair ;
~~~

With this I can easily define `a:dup` to make a copy of an
array.

~~~
:a:dup (a-a)
  here [ dup fetch ,  &, a:for-each ] dip ;
~~~

A closely related word is `a:copy`, which copies an array
from one location to another.

~~~
:a:copy (aa-)
  &Heap [ !Heap dup a:length , &, a:for-each ] v:preserve ;
~~~

`a:to-string` converts an array into a string. The array is
assumed to contain only valid character codes.

~~~
:a:to-string (a-s)
  &Heap [ a:dup #0 , n:inc ] v:preserve s:temp ;
~~~

I then define `a:append` and `a:prepend` to combine arrays.

~~~
:a:append (aa-a)
  dup-pair &fetch bi@ + here [ , [ &, a:for-each ] bi@ ] dip ;

:a:prepend (aa-a)
  swap a:append ;
~~~

`a:chop` returns a new array containing all but the last value
in the source array.

~~~
:a:chop (a-a) a:dup #-1 allot dup v:dec ;
~~~

Next is `a:filter`, which is extracts matching values from
an array. This is used like:

    { #1 #2 #3 #4 #5 #6 #7 #8 }
    [ n:even? ] a:filter

It returns a new array with the values that the quote returned
a `TRUE` flag for.

~~~
:a:filter (aq-)
  [ over &call dip swap &, &drop choose ] curry
  here [ over fetch , a:for-each ] dip
  here over - n:dec over store ;
~~~

Next are `a:contains?` and `a:contains/string?` which
compare a given value to each item in the array and returns
a flag.

~~~
:a:contains? (na-f)
  #0 swap [ swap push over eq? pop or ] a:for-each nip ;

:a:contains/string? (na-f)
  #0 swap [ swap push over s:eq? pop or ] a:for-each nip ;
~~~

I implemented `a:map` to apply a quotation to each item in
an array and construct a new array from the returned values.

Example:

    { #1 #2 #3 }
    [ #10 * ] a:map

~~~
:a:map (aq-a)
  swap [ fetch-next [ [ fetch over call ] sip
                      &store sip n:inc ] times
         drop-pair ] sip ;
~~~

You can use `a:reverse` to make a copy of an array with the
values reversed.

~~~
:a:reverse (a-a)
  here [ fetch-next [ + n:dec ] sip dup ,
         [ dup fetch , n:dec ] times drop
       ] dip ;
~~~

`a:th` provides a quick means of adjusting an array and
offset into an address for use with `fetch` and `store`.

~~~
:a:th (an-a)  + n:inc ;
~~~

I use `a:th` to implement `a:fetch` and `a:store`, for easier
readability.

~~~
:a:fetch (an-v) a:th fetch ;
:a:store (van-) a:th store ;
~~~

~~~
:a:first (a-n) #0 a:fetch ;
:a:last  (a-n) dup a:length n:dec a:fetch ;
~~~

`a:reduce` takes an array, a starting value, and a quote. It
executes the quote once for each item in the array, passing the
item and the value to the quote. The quote should consume both
and return a new value.

~~~
:a:reduce (pnp-n)
  &swap dip a:for-each ;
~~~

~~~
:FREE (-n) STRINGS #1025 - #513 #12 * - here - ;

{{
  'NextArray var
  :arrays FREE here + ;
---reveal---
  :a:temp (a-a) @NextArray dup #12 eq? [ drop #0 dup !NextArray ] if
                #513 * arrays + over a:length n:inc copy
                @NextArray #513 * arrays +
                &NextArray v:inc ;
}}

{{
  'Count var
  :prepare  #0 &Count store ;
  :reserve  swap #0 , ;
  :patch    here over - n:dec over store ;
  :cleanup  dup a:temp swap &Heap store ;
  :record   @Count , ;
  :iterate/n [ over eq? &record if &Count v:inc ] a:for-each ;
  :iterate/s [ over s:eq? &record if &Count v:inc ] a:for-each ;
---reveal---
  :a:indices (av-a)
    prepare here [ reserve iterate/n drop ] dip patch cleanup ;
  :a:indices/string (as-a)
    prepare here [ reserve iterate/s drop ] dip patch cleanup ;
}}
~~~

`a:index` and `a:index/string` build on these to return the
offset of a value in the array, or -1 if the value wasn't
found.

Specifically:

- use `curry` to create a comparator function
- apply this to each item in the array using `a:map`
- find the first TRUE value in the resulting array

The `identify` part of this is a little complex due to me avoiding
using a variable for the flag/offset value, but it's pretty clean
overall.

~~~
:a:index (av-n) [ a:indices #0 a:fetch ] gc ;
:a:index/string (as-n) [ a:indices/string #0 a:fetch ] gc ;
~~~

When making an array, I often want the values in the original
order. The `a:counted-results a:reverse` is a bit long, so
I'm defining a new `a:make` which wraps these.

~~~
:a:make (q-a)
  a:counted-results dup dup &Heap [ a:reverse ] v:preserve swap a:copy ;

:{ (-)  |[ |depth |[ ; immediate
:} (-a) |] |dip |depth |swap |- |n:dec |] |a:make ; immediate
~~~

For comparing arrays, use `a:eq?` or `a:-eq?`.

~~~
:a:hash (a-n) #5381 swap [ swap #33 * + ] a:for-each ;
:a:eq? (aa-f) a:hash swap a:hash eq? ;
:a:-eq? (aa-f) a:hash swap a:hash -eq? ;
~~~

Building on these, I implement `a:begins-with?` and `a:ends-with?`
to determine if an array starts or ends with the values in a
different array.

~~~
:a:begins-with? (aa-f)
  &Heap [ dup a:length &swap dip a:left a:eq? ] v:preserve ;

:a:ends-with? (aa-f)
  &Heap [ dup a:length &swap dip a:right a:eq? ] v:preserve ;
~~~

~~~
{{
  'Substitute d:create #128 allot
  :extract  &Substitute s:copy ;
  :combine  &Substitute s:append s:append ;
  :find-end dup s:length &Substitute s:length - over + ;
  :clean    find-end #0 swap store ;
---reveal---
  :s:replace-all (sss-s)
    &Heap [ extract s:tokenize-on-string s:empty swap
            [ combine ] a:for-each clean ] v:preserve ;
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

And then the `process-tokens` uses `next-token` and `interpret`
to go through the string, processing each token in turn. Using
the standard `interpret` word allows for proper handling of the
sigils and classes so everything works just as if entered
directly.

~~~
  :process-tokens (sn-)
    [ (get_next_token ASCII:SPACE s:split/char ) swap
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
next word, `indexed-times` adds this to RETRO. It also
provides `I`, `J`, and `K` words to access the index of the
current, and up to two outer loops as well.

This supports up to 32 levels of nesting; increase the size
of `Index` if you need more than this.

~~~
{{
  'LP var
  'Index d:create #32 allot
  :next (-) @LP &Index + v:inc ;
  :prep (-) &LP v:inc #0 @LP &Index + store ;
  :done (-) &LP v:dec ;
---reveal---
  :I (-n) @LP &Index + fetch ;
  :J (-n) @LP &Index + n:dec fetch ;
  :K (-n) @LP &Index + #2 - fetch ;
  :indexed-times
    prep swap
      [ repeat 0; \lisupudu `1 \puca.... \popo.... next again ] call
    drop done ;
}}
~~~

## Numeric Bases

~~~
:decimal #10 !Base ;
:binary  #2  !Base ;
:octal   #8  !Base ;
:hex     #16 !Base ;
~~~

~~~
:var-s (ss-) &s:keep dip var-n ;
~~~


## Recursion

`tail-recurse` follows a semicolon, and changes a call right
before the semicolon to a jump.

~~~
:tail-recurse (-) #1793 here #3 - store ;
~~~


### Memory

~~~
:fill (vpn-) [ dup-pair store n:inc ] times drop-pair ;
~~~

## I/O

~~~
:io:enumerate (-n)   \ie...... ;
:io:query     (n-mN) \iq...... ;
:io:invoke    (n-)   \ii...... ;
:io:scan-for  (n-m)
  #-1 swap
  io:enumerate [ I io:query nip over eq?
                 [ [ drop I ] dip ] if ] indexed-times drop ;
~~~

A RETRO system is only required to provide a single I/O word to
the user: a word to push a single character to the output log.
This is always mapped to device 0, and is exposed as `c:put`.

~~~
:c:put (c-) hook #0 io:invoke ;
~~~

This can be used to implement words that push other items to
the log.

~~~
:nl    (-)   ASCII:LF    c:put ;
:sp    (-)   ASCII:SPACE c:put ;
:tab   (-)   ASCII:HT    c:put ;
:s:put (s-)  &c:put s:for-each ;
:n:put (n-)  n:to-string/reversed dup s:length &+ sip n:inc
             [ dup fetch c:put n:dec ] times drop ;
~~~

An interface layer may provide additional I/O words, but these
will not be implemented here as they are not part of the core
language.

## Debugging Aids

I provide a few debugging aids in the core language. The
examples provide much better tools, and interface layers can
provide much more than I can do here.

~~~
:reset      (...-) repeat depth 0; drop-pair again ;
:dump-stack (-)  depth 0; \drpulica ^dump-stack \podulica ^n:put sp ;
~~~

## Listener

The basic image has a space allocated for input at the end of
the kernel. A pointer to this is stored at address 7.

~~~
:TIB #7 fetch ;
~~~

~~~
[ 'ERROR:_Word_Not_Found:_ s:put TIB s:put nl ]
&err:notfound set-hook
~~~

If a VM implementation provides both the character output and a
generic "keyboard" input, the basic listener here can be used.

~~~
:c:get (-c) hook #1 io:scan-for io:invoke ;
:bye (-) \ha...... ;
FALSE 'Ignoring var-n
{{
  'EOT var
  (-nn)  :version    @Version #100 /mod ;
  (c-f)  :done?      dup !EOT
                     [ ASCII:CR eq? ]
                     [ ASCII:LF eq? ]
                     [ ASCII:SPACE eq? ] tri or or ;
  (c-f)  :eol?       @EOT [ ASCII:CR eq? ] [ ASCII:LF eq? ] bi or ;
  (s-sf) :valid?     dup s:length n:strictly-positive? ;
  (c-c)  :check-eof  dup [ #-1 eq? ] [ ASCII:EOT eq? ] bi or &bye if ;
         :bs         buffer:size #2 gteq?
                     [ buffer:get drop ] if buffer:get drop ;
  (c-c)  :check-bs   dup [ ASCII:BS eq? ] [ ASCII:DEL eq? ] bi or &bs if ;
  (c-c)  :check      check-eof check-bs ;
  (-c)   :character  c:get dup buffer:add ;
  (q-)   :buffer     [ TIB buffer:set call buffer:start ] buffer:preserve ;
  (-s)   :read-token [ [ character check done? ] until ] buffer s:chop ;
  (-sf)  :input      read-token valid? ;
  (sf-)  :process    @Ignoring [ drop-pair eol? [ &Ignoring v:off ] if ] if;
                     &interpret &drop choose ;
---reveal---
  :s:get-word (-s) [ #7 fetch buffer:set
                     [ c:get dup buffer:add check-bs done? ] until
                       buffer:start s:chop ] buffer:preserve ;
  :banner  version 'RETRO_12_(%n.%n)\n s:format s:put
           FREE EOM FREE - EOM '%n_Max,_%n_Used,_%n_Free\n s:format s:put ;
  :listen  banner repeat input process again ;
}}

&listen #1 store
~~~

## Dictionary Hashing

In the future, we will be making use of hashes to help improve
the performance of dictionary lookups. The following words can
be used to update the hashes.

In RetroForth/ilo, the hashes are used exclusively. We can't do
that here; keeping name data is needed for some introspection
tasks, and since Nga allows for 64-bit cells, can't guarantee
the constraints will be met. So we'll be bootstrapping using
the names, and patching in the hashes later.

~~~
&s:hash 'd:Hash-Function var-n

:d:rehash (-)
  [ [ d:name @d:Hash-Function call ] sip d:hash store ] d:for-each ;

&d:rehash #10 store
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

    Copyright (c) 2008 - 2021, Charles Childers
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
