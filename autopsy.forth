Autopsy is a debugging tool for RETRO. This is a fresh implementation for RETRO 12 and is intended to be a useful debugging tool.

In implementing this, I identified the core elements I wanted:

- the ability to study memory

  - dumps
  - disassembly

- the ability to edit memory

  - provided by RETRO already via fetch/store and the assembler

- the ability to run a word in a sandbox
- the ability to single step through a word
- the ability to profile instruction frequency
- the ability to watch specific variables and memory locations

This is more ambitious than my prior debuggers. But as I intend to use RETRO 12 for years to come, it'll be a necessary and worthwhile tool.

So some background on the internals.

RETRO runs on a virtual machine called Nga. The instruction set is MISC inspired, consisting of just 27 instructions:

    0  nop        7  jump      14  gt        21  and
    1  lit <v>    8  call      15  fetch     22  or
    2  dup        9  ccall     16  store     23  xor
    3  drop      10  return    17  add       24  shift
    4  swap      11  eq        18  sub       25  zret
    5  push      12  neq       19  mul       26  end
    6  pop       13  lt        20  divmod

Four instructions are packed per 32-bit memory location. The assembler allows the instructions to be specified like:

    'lica.... i
    #100 d

I shorten the instructions to two letter abbreviations, with '..' for 'nop' and then construct a string with all of these. This will be used to resolve names. The ?? at the end will be used for unidentified instructions.

~~~
'..lidudrswpupojucaccreeqneltgtfestadsumudianorxoshzren??
'INST s:const
~~~

Since instructions are packed, I need to unpack them before I can run the individual instructions. I implement `unpack` for this.

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

Now it's possible to write words to display instruction bundles. The formats are kept simple. For a bundle with `lit / lit / add / lit`, this will display either the opcodes (`1,1,17,1`) or a string with the abbreviations (`liliadli`).

~~~
:name-for (n-cc)
  #27 n:min #2 * &INST + fetch-next swap fetch swap ;

:display:bundle<raw> (n-)
  unpack '%n,%n,%n,%n s:with-format puts ;

:display:bundle<named> (n-)
  unpack #4 [ name-for putc putc ] times ;
~~~

So now I'm ready to write a disassembler. I'll provide an output setup like this:

    address 'instructionbundle i
    address #value d [possibly reference]

If the value corresponds to a word in the `Dictionary`, the disassembler will display a message indicating the possible name that corresponds to the value.

To begin, I'll add a variable to track the number of `li` instructions. (These require special handling as they push a value in the following cells to the stack).

~~~
'LitCount var
~~~

I then wrap `name-for` with a simple check that increments `LitCount` as needed.

~~~
:name-for<counting-li> (n-cc)
  dup #1 eq? [ &LitCount v:inc ] if name-for ;
~~~

To actually display a bundle, I need to decide on what it is. So I have a `validate` word to look at each instruction and make sure all are actual instructions.

~~~
:valid? (n-f)
  unpack
  [ #0 #26 n:between? ] bi@ and
  [ [ #0 #26 n:between? ] bi@ and ] dip and ;
~~~

With this and the `LitCount`, I can determine how to render a bundle.

I split out each type (instruction, reference/raw, and data) into a separate handler.

~~~
:render-inst (n-)
  $' putc unpack #4 [ name-for<counting-li> putc putc ] times sp $i putc ;

:render-data (n-)
  $# putc n:to-string puts sp $d putc ;

:render-ref  (n-)
  dup d:lookup-xt n:-zero?
    [ dup render-data tab tab d:lookup-xt d:name '[possibly_`%s`] s:with-format puts ]
    [     render-data ] choose ;
~~~

Then I use these and my `valid?` checker to implement a single word to
render the packed cell in a meaningful manner.

~~~
:render-packed (n-)
  @LitCount n:zero?
  [ dup valid?
    [ render-inst  ]
    [ render-ref ] choose ]
  [ render-ref &LitCount v:dec ] choose ;
~~~

And now to tie it all together:

~~~
:disassemble (an-)
  [ fetch-next
    over putn sp      (address)
    render-packed nl  (inst_or_data)
  ] times drop ;
~~~

Ok, now on to the fun bit: execution trace and single stepping through a word.

This entails writing an implementation of Nga in RETRO. So to start, setup space for the data and address ("return") stacks, as well as variables for the stack pointers and instruction pointer.

~~~
'DataStack d:create  #1024 allot
'ReturnStack d:create  #1024 allot
'SP var
'RP var
'IP var
~~~

Next, helpers to push values from the real stacks to the simulated ones. The stack pointer will point to the next available cell, not the actual top element.

~~~
:to-stack (n-) &DataStack @SP + store &SP v:inc ;
:from-stack (-n) &SP v:dec &DataStack @SP + fetch ;
:to-rstack (n-) &ReturnStack @RP + store &RP v:inc ;
:from-rstack (-n) &RP v:dec &ReturnStack @RP + fetch ;
~~~

One more helper, `[IP]` will return the value in memory at the location `IP` points to.

~~~
:[IP] @IP fetch ;
~~~

Now for the instructions. Taking a cue from the C implementation, I have a separate word for each instruction and then a jump table of addresses that point to these.

~~~
:i:no ;
:i:li  &IP v:inc [IP] to-stack ;
:i:du from-stack dup to-stack to-stack ;
:i:dr from-stack drop ;
:i:sw from-stack from-stack swap to-stack to-stack ;
:i:pu from-stack to-rstack ;
:i:po from-rstack to-stack ;
:i:ju from-stack n:dec !IP ;
:i:ca @IP to-rstack i:ju ;
:i:cc from-stack from-stack [ to-stack i:ca ] [ drop ] choose ;
:i:re from-rstack !IP ;
:i:eq from-stack from-stack eq? to-stack ;
:i:ne from-stack from-stack -eq? to-stack ;
:i:lt from-stack from-stack swap lt? to-stack ;
:i:gt from-stack from-stack swap gt? to-stack ;
:i:fe from-stack fetch to-stack ;
:i:st from-stack from-stack swap store ;
:i:ad from-stack from-stack + to-stack ;
:i:su from-stack from-stack swap - to-stack ;
:i:mu from-stack from-stack * to-stack ;
:i:di from-stack from-stack swap /mod to-stack to-stack ;
:i:an from-stack from-stack and to-stack ;
:i:or from-stack from-stack or to-stack ;
:i:xo from-stack from-stack xor to-stack ;
:i:sh from-stack from-stack swap shift to-stack ;
:i:zr dup n:zero? [ drop i:re ] if ;
:i:en ;
~~~

With the instructions defined, populate the jump table. The order is crucial as the opcode number will be the index into this table.

~~~
'Instructions d:create
  &i:no ,  &i:li ,  &i:du ,  &i:dr ,  &i:sw ,  &i:pu ,  &i:po ,
  &i:ju ,  &i:ca ,  &i:cc ,  &i:re ,  &i:eq ,  &i:ne ,  &i:lt ,
  &i:gt ,  &i:fe ,  &i:st ,  &i:ad ,  &i:su ,  &i:mu ,  &i:di ,
  &i:an ,  &i:or ,  &i:xo ,  &i:sh ,  &i:zr ,  &i:re ,
~~~

With the populated table of instructions, implementing a `process-single-opcode` is easy. This will check the instruction to make sure it's valid, then call the corresponding handler in the instruction table. If not valid, this will report an error.

~~~
:process-single-opcode (n-)
  dup #0 #26 n:between?
  [ &Instructions + fetch call ]
  [ 'Invalid_Instruction:_%n_! s:with-format puts nl ] choose ;
~~~

Next is to unpack an instruction bundle and process each instruction.

~~~
:process-packed-opcode (n-)
  unpack
  process-single-opcode 
  process-single-opcode 
  process-single-opcode 
  process-single-opcode ;
~~~

So the guts of the Nga-in-RETRO are done at this point. Now to implement a method of stepping through execution of a word.

This will display output indicating state. It'll provide:

- current memory location
- values in instruction bundle
- stack depths
- data stack before execution
- data stack after exection

E.g.,

    IP:13966 [19,0,0,0] SP:3 RP:1  Stack: 9 3 3  -> 9 9    

So helpers for displaying things:

~~~
:display-status
  @RP @SP [IP] unpack @IP
  'IP:%n_[%n,%n,%n,%n]\tSP:%n_RP:%n\t s:with-format puts ;

:display-data-stack
  #0 @SP [ &DataStack over + fetch putn sp n:inc ] times drop ;

:display-return-stack
  #0 @RP [ &ReturnStack over + fetch putn sp n:inc ] times drop ;
~~~

And then using the display helpers and instruction processor, a single stepper. (This also updates a `Steps` counter)

~~~
'Steps var

:step (-)
  display-status
  'Stack:_ puts display-data-stack '_->_ puts
  [IP] process-packed-opcode &IP v:inc
  display-data-stack nl
  &Steps v:inc ;
~~~

And then wrap it with `times` to run multiple steps.

~~~
:steps (n-)
  &step times ;
~~~

Then on to the tracer. This will `step` through execution until the word returns. I use a similar approach to how I handle this in the interface layers for RETRO (word execution ends when the address stack depth reaches zero).

The `trace` will empty the step counter and display the number of steps used.

~~~
:trace (a-)
  #0 !Steps
  !IP #0 to-rstack
  [ step @RP n:zero? ] until
  @Steps '%n_steps_taken\n s:with-format puts ;
~~~

==================================================

Tests

~~~
:test
  as{ 'liliaddu i #22 d #33 d }as
  #3 #4 gt? [ #1 ] if ;
&test dup here swap - disassemble
&test trace
~~~

