RETRO runs on a virtual machine called Nga. The instruction set is MISC
inspired, consisting of just 27 instructions:

    0  nop        7  jump      14  gt        21  and
    1  lit <v>    8  call      15  fetch     22  or
    2  dup        9  ccall     16  store     23  xor
    3  drop      10  return    17  add       24  shift
    4  swap      11  eq        18  sub       25  zret
    5  push      12  neq       19  mul       26  end
    6  pop       13  lt        20  divmod

Four instructions are packed per 32-bit memory location. The Muri
assembler allows the instructions to be specified like:

    'lica.... i
    #100      d

I shorten the instructions to two letter abbreviations, with '..' for
'nop'.

The only special case here is that each 'li' instruction requires a
value to push in the following cell.

So to implement this, I first need to unpack a cell into the individual
instructions.

~~~
{{
  :mask #255 and ;
  :next #8 shift ;
---reveal---
  :unpack (n-dcba)
    dup mask swap next
    dup mask swap next
    dup mask swap next
    'abcd 'dcba reorder ;
}}
~~~

With that out of the way, I make a variable to track the number of 'li'
instructions in the bundle.

~~~
'LitCount var
~~~

And then a string with the two letter identifiers for each instruction.
Here I append ?? at the end; I'll use this to identify unknown
instructions.

~~~
'..lidudrswpupojucaccreeqneltgtfestadsumudianorxoshzren?? 'INST s:const
~~~

And then a simple word to return a name for the current instruction. I
have this increment the `LitCount` when it finds a 'li' instruction.

~~~
:name-for (n-cc)
  dup #1 eq? [ &LitCount v:inc ] if
  #27 n:min #2 * &INST + fetch-next swap fetch swap ;
~~~

To actually display a bundle, I need to decide on what it is. So I have
a `validate` word to look at each instruction and make sure all are
actual instructions.

~~~
:valid? (c-f)
  unpack   [ #0 #26 n:between? ] bi@ and
         [ [ #0 #26 n:between? ] bi@ and ] dip and ;
~~~

With this and the `LitCount`, I can determine how to render a bundle.
So the next step is a word to pad raw values so they're at least as
long as instruction bundle strings (8 characters).

~~~
:pad (s-)
  s:length #8 swap - #0 n:max [ sp ] times ;
~~~

I split out each type (instruction, reference/raw, and data) into a
separate handler.

~~~
:render-inst (n-)
  $' putc unpack #4 [ name-for putc putc ] times sp $i putc ;

:render-ref  (n-)
  $# putc n:to-string dup puts pad sp $d putc ;

:render-data (n-)
  $# putc n:to-string dup puts pad &LitCount v:dec sp $d putc ;
~~~

Then I use these and my `valid?` checker to implement a single word to
render the packed cell in a meaningful manner.

~~~
:render-packed (n-)
  @LitCount n:zero?
  [ dup valid?
    [ render-inst ]
    [ render-ref ] choose ]
  [ render-data ] choose ;
~~~

And now to tie it all together:

~~~
:disassemble (an-)
  [ fetch-next
    over $( putc putn sp    (address)
    render-packed nl        (inst_or_data)
  ] times drop ;
~~~

~~~
&disassemble here over - disassemble
