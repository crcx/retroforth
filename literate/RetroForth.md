# RETRO 12

## Background

Retro is a dialect of Forth. It builds on the barebones Rx core,
providing a much more flexible and useful language.

Retro has a history going back many years. It began as a 16-bit
assembly implementation for x86 hardware, evolved into a 32-bit
system with cmForth and ColorForth influences, and eventually started
supporting mainstream OSes. Later it was rewritten for a small,
portable virtual machine. Over the years the language implementation
has varied substantially. This is the twelfth generation of Retro. It
now targets a new virtual machine (called Nga), and is built over a
barebones Forth kernel (called Rx).

### Namespaces

Various past releases have had different methods of dealing with the
dictionary. Retro 12 has a single global dictionary, with a convention
of using a namespace prefix for grouping related words.

| namespace  | words related to   |
| ---------- | ------------------ |
| ASCII      | ASCII Constants    |
| c          | characters         |
| compile    | compiler functions |
| d          | dictionary headers |
| err        | error handlers     |
| n          | numbers            |
| s          | strings            |
| v          | variables          |

### Prefixes

Prefixes are an integral part of Retro. These are single characters
added to the start of a word which indicate to Retro how it should
execute the word. These are processed at the start of interpreting a
token.

| prefix | used for               |
| ------ | ---------------------- |
| :      | starting a definition  |
| &amp;  | obtaining pointers     |
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

This assumes that the VM defines an image as being 524287 cells.

| range           | contains                     |
| --------------- | ---------------------------- |
| 0 - 1024        | rx kernel                    |
| 1025 - 1535     | token input buffer           |
| 1536 +          | start of heap space          |
| 522751          | temporary strings (12 * 128) |
| 524287          | end of memory                |

I provide a word, `EOM`, which returns the last addressable location.
This will be used by the words in the `s:` namespace to allocate the
temporary string buffers at the end of memory.

````
:EOM  (-n)  #-3 fetch ;
````

... stack comments ...

  (takes-returns)

I use a single character for each input and output item. These will
often (though perhaps not always) be:

  n, m, x, y  number
  a, p        pointer
  q           quotation (pointer)
  d           dictionary header (pointer)
  s           string
  c           character (ASCII)

I next define a few words in the `d:` namespace to make it easier
to operate on the most recent header in the dictionary. These return
pointers to specific fields in the header.

````
:d:last        (-d) &Dictionary fetch ;
:d:last<xt>    (-a) d:last d:xt fetch ;
:d:last<class> (-a) d:last d:class fetch ;
:d:last<name>  (-s) d:last d:name ;
````

... reclass ...

This is used to change the class from `class:word` to `class:macro`.
Doing this is ugly and not very readable. I implement `reclass` to
change the class of the most recent word.

````
:reclass    (a-) d:last d:class store ;
````

With this I can then define `immediate` (for state-smart words) and
`data` to tag data words.

````
:immediate  (-)  &class:macro reclass ;
:data       (-)  &class:data reclass ;
````

````
:depth  (-n) #-1 fetch ;
````

````
:prefix:@  (s-n) d:lookup d:xt fetch class:data &fetch class:word ; immediate
:prefix:!  (s-n) d:lookup d:xt fetch class:data &store class:word ; immediate
````

I have a `compile` namespace for some low level words that compile
Nga bytecode.

````
:compile:lit  (a-) #1 , , ;
:compile:jump (a-) #1793 , , ;
:compile:call (a-) #2049 , , ;
:compile:ret  (-)  #10 , ;
````

The compiler state is stored in a value named `Compiler`. I have an
accessor word that aids in readability.

````
:compiling?  (-f)  @Compiler ;
````

It's sometimes useful to inline values directly. I use a backtick
prefix for this.

````
:prefix:`  (s-)
  compiling? [ s:to-number , ] [ drop ] choose ; immediate
````

It's traditional to have a word named `here` which returns the next
free address in memory.

````
:here  (-a) @Heap ;
````

The next few words aren't useful until the `s:` namespace is defined.
With strings and the `'` prefix they allow creation of variables and
constants.

| To create a                  | Use a form like    |
| ---------------------------- | ------------------ |
| Variable                     | `'Base var`        |
| Variable, with initial value | `#10 'Base var<n>` |
| Constant                     | `#-1 'TRUE const`  |

The first word creates a new header pointing to `here`. This is used
to build other data structures without invoking the `:` compiler.

````
:d:create (s-)
  (s-) &class:data #0 d:add-header
  here d:last d:xt store ;
````

And then the others are trivial.

````
:var    (s-)  d:create #0 , ;
:var<n> (ns-) d:create , ;
:const  (ns-) d:create d:last d:xt store ;
````

The `const` word bears a tiny bit of explaination. It takes advantage
of Retro's word class model. It creates a header, with a class of
`class:data`, then sets the word pointer to the value. Since the data
class either leaves the word pointer on the stack or compiles it as
a literal into a definition, this allows constants to exist as just
a header with no special runtime code.

The core Rx language provides a few basic stack shuffling words: `push`,
`pop`, `drop`, `swap`, and `dup`. There are quite a few more that are
useful. Some of these are provided here.

````
:tuck      (xy-yxy)   dup push swap pop ;
:over      (xy-xyx)   push dup pop swap ;
:dup-pair  (xy-xyxy)  over over ;
:nip       (xy-y)     swap drop ;
:drop-pair (nn-)      drop drop ;
:?dup      (n-nn||n-n) dup 0; ;
````

Retro makes use of anonymous functions called *quotations* for much of
the execution flow and stack control. The words that operate on these
quotations are called *combinators*.

`dip` executes a quotation after moving a value off the stack. The
value is restored after execution completes. These are equivilent:

    #10 #12 [ #3 - ] dip
    #10 #12 push #3 - pop

````
:dip  (nq-n)  swap push call pop ;
````

`sip` is similar to dip, but leaves a copy of the value on the stack
while the quotation is executed. These are equivilent:

    #10 [ #3 * ] sip
    #10 dup push #3 * pop

````
:sip  (nq-n)  push dup pop swap &call dip ;
````

Apply each quote to a copy of x

````
:bi  (xqq-)  &sip dip call ;
````

Apply q1 to x and q2 to y

````
:bi*  (xyqq-) &dip dip call ;
````

Apply q to x and y

````
:bi@  (xyq-)  dup bi* ;
````

Apply each quote to a copy of x

````
:tri  (xqqq-)  [ &sip dip sip ] dip call ;
````

Apply q1 to x, q2 to y, and q3 to z

````
:tri*  (xyzqqq-)  [ [ swap &dip dip ] dip dip ] dip call ;
````

Apply q to x, y, and z

````
:tri@ dup dup tri* ;
````

## Flow

Execute quote until quote returns a flag of 0.

````
:while  (q-)
  [ repeat dup dip swap 0; drop again ] call drop ;
````

Execute quote until quote returns a flag of -1.

````
:until  (q-)
  [ repeat dup dip swap #-1 xor 0; drop again ] call drop ;
````

The `times` combinator runs a quote (n) times.

````
:times  (q-)
  swap [ repeat 0; #1 - push &call sip pop again ] call drop ;
````

Taking a break from combinators for a bit, I turn to some words for
comparing things. First, constants for TRUE and FALSE.

````
:TRUE  (-n) #-1 ;
:FALSE (-n)  #0 ;
````

The basic Rx kernel doesn't provide two useful forms which I'll
provide here.

````
:lteq?  (nn-f)  dup-pair eq? [ lt? ] dip or ;
:gteq?  (nn-f)  dup-pair eq? [ gt? ] dip or ;
````

And then some numeric comparators.

````
:n:MAX        (-n)    #2147483647 ;
:n:MIN        (-n)    #−2147483648 ;
:n:zero?      (n-f)   #0 eq? ;
:n:-zero?     (n-f)   #0 -eq? ;
:n:negative?  (n-f)   #0 lt? ;
:n:positive?  (n-f)   #-1 gt? ;
:n:strictly-positive?  (n-f)  #0 gt? ;
:n:even?      (n-f)  #2 /mod drop n:zero? ;
:n:odd?       (n-f)  #2 /mod drop n:-zero? ;
````

And now back to combinators.

`case` is a conditional combinator. It's actually pretty useful. What
it does is compare a value on the stack to a specific value. If the
values are identical, it discards the value and calls a quote before
exiting the word. Otherwise it leaves the stack alone and allows
execution to continue.

Example:

    :c:vowel?
      $a [ TRUE ] case
      $e [ TRUE ] case
      $i [ TRUE ] case
      $o [ TRUE ] case
      $u [ TRUE ] case
      drop FALSE ;

````
:case
  [ over eq? ] dip swap
  [ nip call TRUE ] [ drop FALSE ] choose 0; pop drop drop ;
:s:case
  [ over s:eq? ] dip swap
  [ nip call TRUE ] [ drop FALSE ] choose 0; pop drop drop ;
````

Two more stack shufflers.

`rot` rotates the top three values.

````
:rot  (abc-bca)   [ swap ] dip swap ;
````

Next is `tors`. Short for *top of return stack*, this returns the top
item on the address stack. As an analog to traditional Forth, this is
equivilent to `R@`.

````
:tors (-n)  pop pop dup push swap push ;
````

The core Rx language provides addition, subtraction, multiplication,
and a combined division/remainder. Retro expands on this.

````
:/         (nq-d)  /mod swap drop ;
:mod       (nq-r)  /mod drop ;
:not       (n-n)   #-1 xor ;
:n:pow     (bp-n)  #1 swap [ over * ] times nip ;
:n:negate  (n-n)   #-1 * ;
:n:square  (n-n)   dup * ;
:n:sqrt    (n-n)   #1 [ repeat dup-pair / over - #2 / 0; + again ] call nip ;
:n:min     (nn-n)  dup-pair lt? [ drop ] [ nip ] choose ;
:n:max     (nn-n)  dup-pair gt? [ drop ] [ nip ] choose ;
:n:abs     (n-n)   dup n:negate n:max ;
:n:limit   (nlu-n) swap push n:min pop n:max ;
:n:inc     (n-n)   #1 + ;
:n:dec     (n-n)   #1 - ;
:n:between? (nul-) rot [ rot rot n:limit ] sip eq? ;
````

Some of the above, like `n:inc`, are useful with variables. But it's
messy to execute sequences like:

  @foo n:inc !foo

The `v:` namespace provides words which simplify the overall handling
of variables. With this, the above can become simply:

  &foo v:inc

````
:v:inc-by  (na-)   [ fetch + ] sip store ;
:v:dec-by  (na-)   [ fetch swap - ] sip store ;
:v:inc     (n-n)   #1 swap v:inc-by ;
:v:dec     (n-n)   #1 swap v:dec-by ;
:v:limit   (alu-)  push push dup fetch pop pop n:limit swap store ;
:v:on      (a-)    TRUE swap store ;
:v:off     (a-)    FALSE swap store ;
:v:preserve (aq-)  swap dup fetch [ [ call ] dip ] dip swap store ;
:allot     (n-)    &Heap v:inc-by ;
````

If you need to update a stored variable there are two typical forms:

    #1 'Next var<n>
    @Next #10 * !Next

Or:

    #1 'Next var<n>
    &Next [ fetch #10 * ] sip store

The `v:update-using` replaces this with:

    #1 'Next var<n>
    &Next [ #10 * ] v:update-using

It takes care of preserving the variable address, fetching the stored
value, and updating with the resulting value.

````
:v:update-using (aq-) swap [ fetch swap call ] sip store ;
````

I have a simple word `copy` which copies memory to another location.

````
:copy   (aan-) [ &fetch-next dip store-next ] times drop drop ;
````

Now for something tricky: a system for lexical scoping.

The dictionary is a simple linked list. Retro allows for some control
over what is visible using the `{{`, `---reveal---`, and `}}` words.

As an example:

    {{
      :increment dup fetch n:inc swap store ;
      :Value `0 ;
    ---reveal---
      :next-number @Value &Value increment ;
    }}

Only the `next-number` function will remain visible once `}}` is
executed. 

````
:ScopeList `0 `0 ;
:{{            (-)
  d:last dup &ScopeList store-next store ;
:---reveal---  (-)
   d:last &ScopeList n:inc store ;
:}}            (-)
  &ScopeList fetch-next swap fetch eq?
  [ @ScopeList !Dictionary ]
  [ @ScopeList [ &Dictionary repeat fetch dup fetch &ScopeList n:inc fetch -eq? 0; drop again ] call store ] choose ;
````

--> The scoping code is a bit messy. I'd like to simplify it.


A buffer is a linear memory buffer. Retro provides a `buffer:`
namespace for working with them.

````
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
````

And now for strings. Traditional Forth systems have a messy mix of
strings. You have counted strings, address/length pairs, and sometimes
other forms.

Retro uses zero terminated strings. I know that counted strings are
better in many ways, but I've used these for years and they are a
workable approach.

Temporary strings are allocated in a circular pool (at STRINGS).

````
:TempStrings ;   &class:data reclass  #12 !TempStrings
:TempStringMax ; &class:data reclass #512 !TempStringMax
:STRINGS   EOM @TempStrings @TempStringMax * - ;

{{
  :MAX-LENGTH #512 ;
  :s:Current `0 ; data

  :s:pointer (-p)  @s:Current MAX-LENGTH * STRINGS + ;
  :s:next    (-)
    &s:Current v:inc
    @s:Current @TempStrings eq? [ #0 !s:Current ] if ;
---reveal---
  :s:temp (s-s) dup s:length n:inc s:pointer swap copy s:pointer s:next ;
  :s:empty (-s) s:pointer s:next ;
}}
````

Permanent strings are compiled into memory. To skip over them a helper
function is used. When compiled into a definition this will look like:

    lit &s:skip
    call
    :stringbegins
    .data 98
    .data 99
    .data 100
    .data 0
    lit &stringbegins

The `s:skip` adjusts the Nga instruction pointer to skip to the code
following the stored string.

````
:s:skip (-) pop [ fetch-next n:-zero? ] while n:dec push ;
:s:keep (s-s) compiling? [ &s:skip class:word ] if here [ s, ] dip class:data ;
````

And now a quick `'` prefix. (This will be replaced later). What this
does is either move the string token to the temporary buffer or compile
it into the current definition.

This doesn't support spaces. I use underscores instead. E.g.,

    'Hello_World!

Later in the code I'll add a better implementation which can handle
conversion of _ into spaces.

````
:prefix:' compiling? [ s:keep ] [ s:temp ] choose ; immediate
````

`s:chop` removes the last character from a string.

````
:s:chop (s-s) s:temp dup s:length over + n:dec #0 swap store ;
````

`s:reverse` reverses the order of a string. E.g.,

    'hello'  ->  'olleh'

````
:s:reverse (s-s)
  [ dup s:temp buffer:set &s:length [ dup s:length + n:dec ] bi swap
    [ dup fetch buffer:add n:dec ] times drop buffer:start s:temp ]
  buffer:preserve ;
````

Trimming removes leading (`s:trim-left`) or trailing (`s:trim-right`)
spaces from a string. `s:trim` removes both leading and trailing spaces.

````
:s:trim-left (s-s) s:temp [ fetch-next [ #32 eq? ] [ n:zero? ] bi and ] while n:dec ;
:s:trim-right (s-s) s:temp s:reverse s:trim-left s:reverse ;
:s:trim (s-s) s:trim-right s:trim-left ;
````

`s:prepend` and `s:append` for concatenating strings together.

````
:s:prepend (ss-s)
  s:temp [ dup s:length + [ dup s:length n:inc ] dip swap copy ] sip ;
:s:append (ss-s) swap s:prepend ;
````

`s:for-each` executes a quote once for each cell in string. It is
a key part of building the other high-level string operations.

````
:s:for-each (sq-)
  [ repeat
      over fetch 0; drop
      dup-pair
      [ [ [ fetch ] dip call ] dip ] dip
      [ n:inc ] dip
    again
  ] call drop-pair ;
````

`s:filter` returns a new string, consisting of the characters from
another string that are filtered by a quotation.

    'This_is_a_test [ c:-vowel? ] s:filter

````
:s:filter (sq-s)
  [ s:empty buffer:set swap
    [ dup-pair swap call
        [ buffer:add ]
        [ drop       ] choose
    ] s:for-each drop buffer:start
  ] buffer:preserve ;
````

`s:map` Return a new string resulting from applying a quotation to each
character in a source string.

    'This_is_a_test [ $_ [ ASCII:SPACE ] case ] s:map

````
:s:map (sq-s)
  [ s:empty buffer:set swap
    [ over call buffer:add ]
    s:for-each drop buffer:start
  ] buffer:preserve ;
````

`s:substr` returns a subset of a string. Provide it with a string,
a starting offset, and a length.

````
:s:substr (sfl-s)
  [ + s:empty ] dip [ over [ copy ] dip ] sip
  over [ + #0 swap store ] dip ;
````

`s:right` and `s:left` are similar to `s:substr`, but operate
from fixed ends of the string.

````
:s:right (sn-s) over s:length over - swap s:substr ;
:s:left  (sn-s) #0 swap s:substr ;
````

Hash (using DJB2)

````
:s:hash (s-n) #5381 swap [ swap #33 * + ] s:for-each ;
````

Copy a string, including the terminator.

````
:s:copy (ss-) over s:length n:inc copy ;
````

````
:s:DIGITS          (-s)  '0123456789 ;
:s:ASCII-LOWERCASE (-s)  'abcdefghijklmnopqrstuvwxyz ;
:s:ASCII-UPPERCASE (-s)  'ABCDEFGHIJKLMNOPQRSTUVWXYZ ;
:s:ASCII-LETTERS   (-s)  'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ;
:s:PUNCTUATION     (-s)  '_!"#$%&'()*+,-./:;<=>?@[\]^`{|}~ $_ over store ;
's:WHITESPACE d:create  #32, #9 , #10 , #13 , #0 ,
````

Not all characters can be obtained via the $ prefix. ASCII has many
characters that aren't really intended to be printable. Retro has an
`ASCII` namespace providing symbolic names for these.

Note that `ASCII:HT` is the horizontal tab character.

````
:ASCII:NUL     (-c)  #0 ;    :ASCII:SOH     (-c)  #1 ;
:ASCII:STX     (-c)  #2 ;    :ASCII:ETX     (-c)  #3 ;
:ASCII:EOT     (-c)  #4 ;    :ASCII:ENQ     (-c)  #5 ;
:ASCII:ACK     (-c)  #6 ;    :ASCII:BEL     (-c)  #7 ;
:ASCII:BS      (-c)  #8 ;    :ASCII:HT      (-c)  #9 ;
:ASCII:LF      (-c)  #10 ;   :ASCII:VT      (-c)  #11 ;
:ASCII:FF      (-c)  #12 ;   :ASCII:CR      (-c)  #13 ;
:ASCII:SO      (-c)  #14 ;   :ASCII:SI      (-c)  #15 ;
:ASCII:DLE     (-c)  #16 ;   :ASCII:DC1     (-c)  #17 ;
:ASCII:DC2     (-c)  #18 ;   :ASCII:DC3     (-c)  #19 ;
:ASCII:DC4     (-c)  #20 ;   :ASCII:NAK     (-c)  #21 ;
:ASCII:SYN     (-c)  #22 ;   :ASCII:ETB     (-c)  #23 ;
:ASCII:CAN     (-c)  #24 ;   :ASCII:EM      (-c)  #25 ;
:ASCII:SUB     (-c)  #26 ;   :ASCII:ESC     (-c)  #27 ;
:ASCII:FS      (-c)  #28 ;   :ASCII:GS      (-c)  #29 ;
:ASCII:RS      (-c)  #30 ;   :ASCII:US      (-c)  #31 ;
:ASCII:SPACE   (-c)  #32 ;   :ASCII:DEL     (-c)  #127 ;
````

These words operate on character values. Retro currently deals with
ASCII, though cells are 32 bits in length, so Unicode values can be
stored.

First are a bunch of words to help identify character values.

````
:c:letter?      (c-f) $A $z n:between? ;
:c:lowercase?   (c-f) $a $z n:between? ;
:c:uppercase?   (c-f) $A $Z n:between? ;
:c:digit?       (c-f) $0 $9 n:between? ;
:c:whitespace?  (c-f)
  ASCII:SPACE [ TRUE ] case
  ASCII:HT    [ TRUE ] case
  ASCII:LF    [ TRUE ] case
  ASCII:CR    [ TRUE ] case
  drop FALSE ;
:c:visible?     (c-f) #31 #126 n:between? ;
:c:vowel?       (c-f)
  $a [ TRUE ] case
  $e [ TRUE ] case
  $i [ TRUE ] case
  $o [ TRUE ] case
  $u [ TRUE ] case
  $A [ TRUE ] case
  $E [ TRUE ] case
  $I [ TRUE ] case
  $O [ TRUE ] case
  $U [ TRUE ] case
  drop FALSE ;
:c:consonant?   (c-f)
  dup c:letter? [ c:vowel? not ] [ drop FALSE ] choose ;
````

And the inverse forms. (These are included for readability and
orthiginal completion).

````
:c:-lowercase?  (c-f) c:lowercase? not ;
:c:-uppercase?  (c-f) c:uppercase? not ;
:c:-digit?      (c-f) c:digit? not ;
:c:-whitespace? (c-f) c:whitespace? not ;
:c:-visible?    (c-f) c:visible? not ;
:c:-vowel?      (c-f)  c:vowel? not ;
:c:-consonant?  (c-f)  c:consonant? not ;
````

The next few words perform simple transformations.

````
:c:to-upper     (c-c) dup c:lowercase? 0; drop ASCII:SPACE - ;
:c:to-lower     (c-c) dup c:uppercase? 0; drop ASCII:SPACE + ;
:c:toggle-case  (c-c) dup c:lowercase? [ c:to-upper ] [ c:to-lower ] choose ;
:c:to-string    (c-s) '. s:temp [ store ] sip ;
````

With the character transformations a few more string words are
possible.

````
:s:to-upper  (s-s)  [ c:to-upper ] s:map ;
:s:to-lower  (s-s)  [ c:to-lower ] s:map ;
````

Convert a decimal (base 10) number to a string.

````
{{
  :Value `0 ;
  :correct (c-c)
    dup $0 lt? [ $0 over - #2 * + ] if ; 
---reveal---
  :n:to-string  (n-s)
    [ here buffer:set dup !Value n:abs
      [ #10 /mod swap $0 + correct buffer:add dup n:-zero? ] while drop
      @Value n:negative? [ $- buffer:add ] if
      buffer:start s:reverse s:temp ] buffer:preserve ;
}}
````

Now replace the old prefix:' with this one that can optionally turn
underscores into spaces.

````
TRUE 'RewriteUnderscores var<n>

{{
  :sub (c-c) $_ [ ASCII:SPACE ] case ;
  :rewrite (s-s)
    @RewriteUnderscores [ [ sub ] s:map ] if &prefix:' call ;
---reveal---
  :prefix:' rewrite ; immediate
}}
````

Building on `s:for-each`, I am able to implement `s:index-of`, which
finds the first instance of a character in a string.

````
:s:index-of (sc-n)
  swap [ repeat
           fetch-next 0; swap
           [ over -eq? ] dip
           swap 0; drop
         again
       ] sip
  [ - n:dec nip ] sip
  s:length over eq? [ drop #-1 ] if ;
````

`s:contains-char?` returns a flag indicating whether or not a given
character is in a string.

````
:s:contains-char? (sc-f) s:index-of #-1 -eq? ;
````

`s:contains-string?` returns a flag indicating whether or not a given
substring is in a string.

````
{{
  'Src var
  'Tar var
  'Pad var
  'I   var
  'F   var

  :terminate (-)
    #0 @Pad @Tar s:length + store ;

  :extract (-)
    @Src @I + @Pad @Tar s:length copy ;

  :compare (-)
    @Pad @Tar s:eq? @F or !F ;

  :next (-)
    &I v:inc ;
---reveal---
  :s:contains-string? (ss-f)
    !Tar !Src s:empty !Pad #0 !I #0 !F
    @Src s:length
    [ extract terminate compare next ] times
    @F ;
}}
````

The `s:split` splits a string on the first instance of a given
character. Results are undefined if the character can not be
located.

````
:s:split (sc-ss)
  dup-pair s:index-of nip dup-pair s:left [ + ] dip ;
````

Ok, This is a bit of a hack, but very useful at times.

Assume you have a bunch of values:

    #3 #1 #2 #5

And you want to reorder them into something new:

    #1 #3 #5 #5 #2 #1

Rather than using a lot of shufflers, `reorder` simplfies this into:

    #3 #1 #2 #5
    'abcd  'baddcb reorder

````
{{
  'Values var #27 allot
  :from s:length dup [ [ &Values + store ] sip n:dec ] times drop ;
  :to dup s:length [ fetch-next $a -  n:inc &Values + fetch swap ] times drop ;
---reveal---
  :reorder (...ss-?) [ from ] dip to ;
}}
````

I need to describe these and provide some examples of where they are
useful.

````
:curry (vp-p) here [ swap compile:lit compile:call compile:ret ] dip ;
:does  (q-)   d:last<xt> swap curry d:last d:xt store &class:word reclass ;
````

`d:for-each` is a combinator which runs a quote once for each header in
the dictionary. A pointer to each header will be passed to the quote as
it is run.

````
:d:for-each (q-)
  &Dictionary [ repeat fetch 0;
 dup-pair [ [ swap call ] dip ] dip again ] call drop ;
````

Use `s:with-format` to construct a string from multiple items. This
can be illustrated with:

    #4 #6 #10  '%n-%n=%n\n  s:with-format

The format language is simple:

| \n | Replace with a LF                         |
| \t | Replace with a TAB                        |
[ \\ | Replace with a single \                   |
| %c | Replace with a character on the stack     |
| %s | Replace with a string on the stack        |
| %n | Replace with the next number on the stack |

````
{{
  :char (c-)
    $n [ ASCII:LF buffer:add ] case
    $t [ ASCII:HT buffer:add ] case
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
  :s:with-format (...s-s)
    [ s:empty [ buffer:set
      [ repeat fetch-next 0; handle again ]
      call drop ] sip ] buffer:preserve ;
}}
````

## Sets

Sets are statically sized arrays. They are represented in memory as:

    count
    data #1 (first)
    ...
    data #n (last)

Since the count comes first, a simple `fetch` will suffice to get it,
but for completeness (and to allow for future changes), we wrap this
as `set:length`:

````
:set:length (a-n) fetch ;
````

The first couple of words are used to create sets. The first,
`set:from-results` executes a quote and constructs a set from the
returned values.

````
:set:from-results (q-a)
  depth [ call ] dip depth swap -
  here [ dup , [ , ] times ] dip ;
````

The second, `set:from-string`, creates a new string with the characters
in given a string.

````
:set:from-string (s-a)
  s:reverse [ [ ] s:for-each ] curry
  set:from-results ;
````

A very crucial piece is `set:for-each`. This runs a quote once against
each value in a set. This will be leveraged to implement additional
combinators.

````
{{
  'Q var
---reveal---
  :set:for-each (aq-)
    @Q [ !Q fetch-next
         [ fetch-next swap [ @Q call ] dip ] times drop
       ] dip !Q ;
}}
````

With this I can easily define `set:dup` to make a copy of a set.

````
:set:dup (a-a)
  here [ dup fetch , [ , ] set:for-each ] dip ;
````

Next is `set:filter`, which is extracts matching values from a set. This
is used like:

    [ #1 #2 #3 #4 #5 #6 #7 #8 ] set:from-results
    [ n:even? ] set:filter

It returns a new set with the values that the quote returned a `TRUE`
flag for.

````
:set:filter (aq-)
  [ over [ call ] dip swap [ , ] [ drop ] choose ] curry
  here [ over fetch , set:for-each ] dip here over - n:dec over store ;
````

Next are `set:contains?` and `set:contains-string?` which compare a given
value to each item in the array and returns a flag.

````
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
````

I implemented `set:map` to apply a quotation to each item in a set and
construct a new set from the returned values.

Example:

    [ #1 #2 #3 ] set:from-results
    [ #10 * ] set:map

````
:set:map (aq-a)
  [ call , ] curry
  here [ over fetch , set:for-each ] dip ;
````

You can use `set:reverse` to make a copy of a set with the values
reversed. This can be useful after a `set:from-results`.

````
:set:reverse (a-a)
  here [ fetch-next [ + n:dec ] sip dup ,
         [ dup fetch , n:dec ] times drop
       ] dip ;
````

`set:nth` provides a quick means of adjusting a set and offset into an
address for use with `fetch` and `store`.

````
:set:nth (an-a)
  + n:inc ;
````

`set:reduce` takes a set, a starting value, and a quote. It executes
the quote once for each item in the set, passing the item and the value
to the quote. The quote should consume both and return a new value.

````
:set:reduce (pnp-n)
  [ swap ] dip set:for-each ;
````

## Muri: an assembler

Muri is my minimalist assembler for Nga. This is an attempt to
implement something similar in Retro.

This is kept in the global namespace, but several portions are
kept private.

````
{{
````

I allocate a small buffer for each portion of an instruction
bundle.

````
'I0 d:create #3 allot
'I1 d:create #3 allot
'I2 d:create #3 allot
'I3 d:create #3 allot
````

The `opcode` word maps a two character instruction to an opcode
number.

````
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
  'en [ #26 ] s:case  drop #0 ;
````

I use `pack` to combine the individual parts of the instruction
bundle into a single cell.

````
:pack (-n)
  &I0 opcode
  &I1 opcode
  &I2 opcode
  &I3 opcode
  #-24 shift  swap
  #-16 shift + swap
  #-8  shift + swap + ;
````

Switch to the public portion of the code.

````
---reveal---
````

With this it's pretty easy to implement the instruction bundle
handler. Named `i`, this takes a string with four instruction
names, splits it into each part, calls `opcode` on each and
then `pack` to combine them before using `,` to write them into
the `Heap`.

````
:i (s-)
  dup &I0 #2 copy #2 +
  dup &I1 #2 copy #2 +
  dup &I2 #2 copy #2 +
      &I3 #2 copy
  pack , ;
````

The `d` word inlines a data item.

````
:d (n-)
  , ;
````

And `r` inlines a reference (pointer).

````
:r (s-)
  d:lookup d:xt fetch , ;
````

The final bits are `as{` and `}as`, which start and stop the
assembler. (Basically, they just turn the `Compiler` on and
off, restoring its state as needed).

````
:as{ (-f)
  @Compiler &Compiler v:off ; immediate

:}as (f-?)
  !Compiler ; immediate
````

This finishes by sealing off the private words.

````
}}
````

## Evaluating Source

The standard interfaces have their own approaches to getting and
dealing with user input. Sometimes though it'd be nicer to have a
way of evaluating code from within RETRO itself. This provides an
`evaluate` word.

````
{{
````

First, create a buffer for the string to be evaluated. This is sized
to allow for a standard FORTH block to fit, or to easily fit a RETRO
style 512 character block. It's also long enough for most source lines
I expect to encounter when working with files.

````
  'Current-Line d:create
    #1025 allot
````

To make use of this, we need to know how many tokens are in the input
string. The `count-tokens` word will do this, by filtering out anything
other than spaces and getting the size of the remaining string.

````
  :count-tokens (s-n)
    [ ASCII:SPACE eq? ] s:filter s:length ;
````

The `next-token` word splits the remainimg string on SPACE and returns
both parts.

````
  :next-token (s-ss)
    ASCII:SPACE s:split ;
````

And then the `process-tokens` uses `next-token` and `interpret` to go
through the string, processing each token in turn. Using the standard
`interpret` word allows for proper handling of the prefixes and classes
so everything works just as if entered directly.

````
  :process-tokens (sn-)
    [ next-token swap
      [ dup s:length n:-zero? [ interpret ] [ drop ] choose ] dip n:inc
    ] times interpret ;
````

````
---reveal---
````

And finally, tie it all together into the single exposed word
`evaluate`.

````
  :s:evaluate (s-...)
    &Current-Line s:copy
    &Current-Line dup count-tokens process-tokens ;
````

````
}}
````


## I/O

Retro really only provides one I/O function in the standard interface:
pushing a character to the output log.

````
:putc (c-) `1000 ;
````

This can be used to implement words that push other item to the log.

````
:nl   (-)  ASCII:LF putc ;
:sp   (-)  ASCII:SPACE putc ;
:tab  (-)  ASCII:HT putc ;
:puts (s-) [ putc ] s:for-each ;
:putn (n-) n:to-string puts ;
````

Different inteface layers may provide additional I/O words.

## Debugging Aids

I provide just a few debugging aids.

````
:words      (-)  [ d:name puts sp ] d:for-each ;
:reset      (...-) depth repeat 0; push drop pop #1 - again ;
:dump-stack (-)  depth 0; drop push dump-stack pop dup putn sp ;
````

## The End

## Legalities

Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted, provided that the
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

    Copyright (c) 2008 - 2017, Charles Childers
    Copyright (c) 2012 - 2013, Michal J Wallace
    Copyright (c) 2009 - 2011, Luke Parrish
    Copyright (c) 2009 - 2010, JGL
    Copyright (c) 2010 - 2011, Marc Simpson
    Copyright (c) 2011 - 2012, Oleksandr Kozachuk
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
