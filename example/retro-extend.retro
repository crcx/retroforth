#!/usr/bin/env retro

There are two parts to building a Retro image. The first is the
assembly of the kernel using `retro-muri`. The second is to use
the kernel words and build up a complete image. I use this tool
(or an equivilent in C) for this.

# Unu

Since sources are written in a literate format I have a version
of the `retro-unu` tool included here. This will run a quote on
each line in the source that is a fenced region.

~~~
{{
  'Fenced var
  :toggle-fence @Fenced not !Fenced ;
  :fenced? (-f) @Fenced ;
  :handle-line (s-)
    fenced? [ over call ] [ drop ] choose ;
---reveal---
  :unu (sq-)
    swap [ dup '~~~ s:eq?
           [ drop toggle-fence ]
           [ handle-line       ] choose
         ] file:for-each-line drop ;
}}
~~~

# Nga in Retro

Since this is written in Retro and builds a new image, I need
to keep the two instances separate. To do this, I implement a
version of the Nga virtual machine in Retro. This will execute
code in the new image.

So first, allocate a memory region for the new image and
stacks. I also create variables to hold the instruction and
stack pointers.

~~~
#65535 #4 * 'IMAGE-SIZE const

'Image       d:create  IMAGE-SIZE allot
'DataStack   d:create  #1024 allot
'ReturnStack d:create  #4096 allot
'SP var
'RP var
'IP var
~~~

There are a few items in the kernel I need to access as this
progresses. I will fill in the value for `interpret` later.

~~~
#1025 &Image + 'TIB const
#367 't:interpret var<n>
#339 't:notfound var<n>
~~~

I next define helpers to move values to/from the host data
stack to the target ones.

~~~
:>s  (n-) &DataStack @SP + store &SP v:inc ;
:s>  (-n) &SP v:dec &DataStack @SP + fetch ;
:>r  (n-) &ReturnStack @RP + store &RP v:inc ;
:r>  (-n) &RP v:dec &ReturnStack @RP + fetch ;
~~~

One more helper here: a word to return the value that the
`IP` register points to in the target memory.

~~~
:[IP] @IP &Image + fetch ;
~~~

Ok, now for the instructions. See the Nga documentation
for these. Basically I just move things to/from the target
stacks, use the host words, then push the updated values
back.

~~~
:i:no                            ;
:i:li       &IP v:inc [IP] >s    ;
:i:du s>    dup            >s >s ;
:i:dr s>    drop                 ;
:i:sw s> s> swap           >s >s ;
:i:pu s>    >r                   ;
:i:po       r>             >s    ;
:i:ju s>    n:dec !IP            ;
:i:ca       @IP >r i:ju          ;
:i:cc s> s> [ >s i:ca ] [ drop ] choose ;
:i:re       r> !IP               ;
:i:eq s> s> eq?            >s    ;
:i:ne s> s> -eq?           >s    ;
:i:lt s> s> swap lt?       >s    ;
:i:gt s> s> swap gt?       >s    ;
:i:fe s>    #-1 [ @SP >s ] case
            #-2 [ @RP >s ] case
            #-3 [ IMAGE-SIZE >s ] case
            &Image + fetch >s    ;
:i:st s> s> swap &Image + store  ;
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

As with the C implementation, I use a jump table to map the
instructions to their handlers.

~~~
'Instructions d:create
  &i:no ,  &i:li ,  &i:du ,  &i:dr ,  &i:sw ,  &i:pu ,
  &i:po ,  &i:ju ,  &i:ca ,  &i:cc ,  &i:re ,  &i:eq ,
  &i:ne ,  &i:lt ,  &i:gt ,  &i:fe ,  &i:st ,  &i:ad ,
  &i:su ,  &i:mu ,  &i:di ,  &i:an ,  &i:or ,  &i:xo ,
  &i:sh ,  &i:zr ,  &i:en ,  &i:ie ,  &i:iq ,  &i:ii ,
~~~

Now to actually process the instructions. Instructions are
packed, so I need a word to unpack them. This is a simple
matter of shifting and masking.

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

Processing of a single opcode is next. This will do some
validation to make sure the opcode is in the expected range.

~~~
:process-single-opcode (n-)
  dup #0 #29 n:between?
  [ 0; &Instructions + fetch call ]
  [ 'Invalid_Instruction:_%n_! s:format s:put nl ] choose ;
~~~

And then a word to process a packed opcode. This also traps
the `err:notfound` to report on word-not-found conditions.

~~~
:notfound? (-f) @IP @t:notfound eq? ;
:display   (-)  #1025 &Image + s:put sp $? c:put nl ;
:process-packed-opcode (n-)
  notfound? [ display ] if
  unpack
  process-single-opcode 
  process-single-opcode 
  process-single-opcode 
  process-single-opcode ;
~~~

The final part of running code in the target is the
`execute` word. This will run through code until the
top level word called returns.

~~~
:execute (a-)
  !IP #0 >r
  [ [IP] process-packed-opcode &IP v:inc
    @RP n:zero? ] until ;
~~~

# Load the Kernel

~~~
'FID var

:read-byte (n-)  @FID file:read #255 and ;

:read-cell (-n)
  read-byte    read-byte    read-byte  read-byte
  #-8 shift +  #-8 shift +  #-8 shift + ;

:size (-n) @FID file:size #4 / ;

:load-image (s-)
  file:R file:open !FID
  &Image size [ read-cell over store n:inc ] times drop
  size n:put '_cells_loaded s:put nl
  @FID file:close ;

'ngaImage load-image
~~~

# Map in Functions

~~~
:image:Dictionary &Image #2 + ;

:xt-for (s-a)
  here store
  image:Dictionary fetch &Image +
  [ repeat fetch 0; dup d:name here fetch s:eq?
    [ dup d:xt fetch here n:inc store ] if again ] call
  here n:inc fetch ;

:map (as-)
  dup '__`%s`... s:format s:put
  xt-for over store
  '_@_ s:put fetch n:put nl ;

'Find: s:put nl
&t:interpret 'interpret map
&t:notfound  'err:notfound map
~~~

# Process the Extensions

~~~
'Process_Tokens s:put nl

:gc      (a-)   &Heap swap v:preserve ;
:to-TIB  (s-)   TIB s:copy ;
:process        #1025 >s @t:interpret execute ;
:valid?  (s-sf) dup s:length n:-zero? ;
:progress       $. c:put ;

#0 sys:argv
  [ [ ASCII:SPACE s:tokenize
      [ valid? [ to-TIB process ] &drop choose ] a:for-each
      progress
    ] gc ] unu nl
~~~

# Save the Image

~~~
'FID var

:write-byte (n-)  @FID file:write ;
:mask       (n-)  #255 and ;

:write-cell (n-)
           dup mask write-byte
  #8 shift dup mask write-byte
  #8 shift dup mask write-byte
  #8 shift     mask write-byte ;

:save-image (s-)
  file:W file:open !FID
  &Image &Image #3 + fetch [ fetch-next write-cell ] times drop
  &Image #3 + fetch n:put sp 'cells_written s:put nl
  @FID file:close ;

'ngaImage2 save-image
~~~
