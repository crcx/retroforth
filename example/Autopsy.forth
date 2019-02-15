                 _                        
      __ _ _   _| |_ ___  _ __  ___ _   _ 
     / _` | | | | __/ _ \| '_ \/ __| | | |
    | (_| | |_| | || (_) | |_) \__ | |_| |
     \__,_|\__,_|\__\___/| .__/|___/\__, |
                         |_|        |___/ 

Autopsy is a set of debugging tools for RETRO.

# Background

RETRO runs on a virtual machine called Nga. The instruction set is MISC inspired, consisting of just 30 instructions:

    0  nop       10  return     20  divmod
    1  lit <v>   11  eq         21  and
    2  dup       12  neq        22  or
    3  drop      13  lt         23  xor
    4  swap      14  gt         24  shift
    5  push      15  fetch      25  zret
    6  pop       16  store      26  end
    7  jump      17  add        27  ienumerate
    8  call      18  subtract   28  iquery
    9  ccall     19  multiply   29  iinvoke

The first two characters of each instruction name are sufficient to identify the instruction.

Nga exposes memory as an array of 32-bit signed integers. Each memory location can store four instructions. The assembler expects the instructions to be named using their two character identifiers. E.g.,

    'lica.... i
    #100 d

# Disassembly

I use  '..' for 'no(p)' and then construct a string with all of these. This will be used to resolve names. The ?? at the end will be used for unidentified instructions.

~~~
'..lidudrswpupojucaccreeqneltgtfestadsumudianorxoshzrenieiqii??
'INST s:const
~~~

Since instructions are packed, I need to unpack them before I can run or display the individual instructions. I implement `unpack` for this.

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
  #30 n:min #2 * &INST + fetch-next swap fetch swap ;

:display:bundle<raw> (n-)
  unpack '%n,%n,%n,%n s:format s:put ;

:display:bundle<named> (n-)
  unpack #4 [ name-for c:put c:put ] times ;
~~~

So now I'm ready to write an actual disassembler. I'll provide an output setup like this:

    (address) 'instructionbundle i
    (address) #value d (possibly_`reference`)

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
  [ #0 #29 n:between? ] bi@ and
  [ [ #0 #29 n:between? ] bi@ and ] dip and ;
~~~

With this and the `LitCount`, I can determine how to render a bundle.

I split out each type (instruction, reference/raw, and data) into a separate handler.

~~~
:render-inst (n-)
  $' c:put unpack #4 [ name-for<counting-li> c:put c:put ] times sp $i c:put ;

:render-data (n-)
  $# c:put n:to-string s:put sp $d c:put ;

:render-ref  (n-)
  dup d:lookup-xt n:-zero?
    [ dup render-data
      d:lookup-xt d:name '\t\t(possibly\_`%s`) s:format s:put ]
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
'TryToIdentifyWords var

:disassemble (an-)
  #0 !LitCount
  [
    @TryToIdentifyWords
    [ dup d:lookup-xt n:-zero?
      [ dup d:lookup-xt d:name nl s:put nl ] if ] if
    fetch-next
    over $( c:put n:put $) c:put sp (address)
    render-packed nl  (inst_or_data)
  ] times drop ;
~~~

# Execution Trace and Single Stepper

Ok, now on to the fun bit: execution trace and single stepping through a word.

This entails writing an implementation of Nga in RETRO. So to start, setup space for the data and address ("return") stacks, as well as variables for the stack pointers and instruction pointer.

~~~
'DataStack d:create    #128 allot
'ReturnStack d:create  #768 allot
'SP var
'RP var
'IP var
~~~

Next, helpers to push values from the real stacks to the simulated ones. The stack pointer will point to the next available cell, not the actual top element.

~~~
:>s (n-) &DataStack @SP + store &SP v:inc ;
:s> (-n) &SP v:dec &DataStack @SP + fetch ;
:>r (n-) &ReturnStack @RP + store &RP v:inc ;
:r> (-n) &RP v:dec &ReturnStack @RP + fetch ;
~~~

One more helper, `[IP]` will return the value in memory at the location `IP` points to.

~~~
:[IP] @IP fetch ;
~~~

Now for the instructions. Taking a cue from the C implementation, I have a separate word for each instruction and then a jump table of addresses that point to these.

~~~
:i:no                            ;
:i:li       &IP v:inc [IP] >s    ;
:i:du s>    dup            >s >s ;
:i:dr s>    drop                 ;
:i:sw s> s> (swap           >s >s ;
:i:pu s>    >r                   ;
:i:po       r>             >s    ;
:i:ju s>    n:dec !IP            ;
:i:ca       @IP >r i:ju          ;
:i:cc s> s> nl dump-stack nl [ >s i:ca ] [ drop ] choose ;
:i:re       r> !IP               ;
:i:eq s> s> eq?            >s    ;
:i:ne s> s> -eq?           >s    ;
:i:lt s> s> swap lt?       >s    ;
:i:gt s> s> swap gt?       >s    ;
:i:fe s>    fetch          >s    ;
:i:st s> s> swap store           ;
:i:ad s> s> +              >s    ;
:i:su s> s> swap -         >s    ;
:i:mu s> s> *              >s    ;
:i:di s> s> swap /mod swap >s >s ;
:i:an s> s> and            >s    ;
:i:or s> s> or             >s    ;
:i:xo s> s> xor            >s    ;
:i:sh s> s> swap shift     >s    ;
:i:zr s>    dup n:zero? [ drop i:re ] [ >s ] choose ;
:i:en       #0 !RP               ;
:i:ie       #1             >s    ;
:i:iq       #0 dup         >s >s ;
:i:ii s> s> nip c:put            ;
~~~

With the instructions defined, populate the jump table. The order is crucial as the opcode number will be the index into this table.

~~~
'Instructions d:create
  &i:no ,  &i:li ,  &i:du ,  &i:dr ,  &i:sw ,  &i:pu ,
  &i:po ,  &i:ju ,  &i:ca ,  &i:cc ,  &i:re ,  &i:eq ,
  &i:ne ,  &i:lt ,  &i:gt ,  &i:fe ,  &i:st ,  &i:ad ,
  &i:su ,  &i:mu ,  &i:di ,  &i:an ,  &i:or ,  &i:xo ,
  &i:sh ,  &i:zr ,  &i:en ,  &i:ie ,  &i:iq ,  &i:ii ,
~~~

With the populated table of instructions, implementing a `process-single-opcode` is easy. This will check the instruction to make sure it's valid, then call the corresponding handler in the instruction table. If not valid, this will report an error.

~~~
:process-single-opcode (n-)
  dup #0 #29 n:between?
  [ &Instructions + fetch call ]
  [ 'Invalid_Instruction:_%n_! s:format s:put nl ] choose ;
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

    IP:13966 SP:3 RP:1
      [19,0,0,0] - mu......
      Stack: 9 3 3  -> 9 9    

So helpers for displaying things:

~~~
:display-status
  @RP @SP @IP 'IP:%n_SP:%n_RP:%n\n s:format s:put
  [IP] [ unpack ] sip '__%n_->_[%n,%n,%n,%n]_->_ s:format s:put
  [IP] unpack #4 [ name-for<counting-li> c:put c:put ] times nl ;

:display-data-stack
  #0 @SP [ &DataStack over + fetch n:put sp n:inc ] times drop ;

:display-return-stack
  #0 @RP [ &ReturnStack over + fetch n:put sp n:inc ] times drop ;
~~~

And then using the display helpers and instruction processor, a single stepper. (This also updates a `Steps` counter)

~~~
'Steps var

:step (-)
  @IP d:lookup-xt n:-zero? [ @IP d:lookup-xt d:name nl tab s:put nl ] if
  display-status
  @IP n:inc fetch sp sp n:put nl
  @IP #2 + fetch sp sp n:put nl
  '__Stack:_ s:put display-data-stack '_->_ s:put
  [IP] process-packed-opcode &IP v:inc
  display-data-stack nl nl
  &Steps v:inc ;
:astep
  [IP] process-packed-opcode &IP v:inc
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
  !IP #0 >r
  [ step @RP n:zero? @IP n:negative? or ] until
  nl @Steps '%n_steps_taken\n s:format s:put ;
~~~

# Tests

```
:test
  as{ 'liliaddu i #22 d #33 d }as
  #3 #4 gt? [ #1 ] if ;

#0 #100 disassemble
nl '-------------------------- s:put nl
&TryToIdentifyWords v:on
#0 #100 disassemble
```
