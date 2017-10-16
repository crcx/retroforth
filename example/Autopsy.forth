# example|Autopsy

Autopsy is a debugging tool for Retro.

Declare module constant (prevents reloading with multiple `import` statements)

````
:example|Autopsy ;
````

Import dependencies:

````
'FormattedStringOutput import:module
'Strings import:module
````

----

This module provides words to aid in debugging your code.

````
{{
  here #128 allot 'NEEDLE const
---reveal---
  :words<in-namespace> (s-)
    dup '\n%s_contains:\n putsf
    NEEDLE over s:length n:inc copy
    &Dictionary
    repeat
      fetch 0;
      dup d:name #0 NEEDLE s:length s:substr
      NEEDLE s:eq? [ dup d:name NEEDLE s:length + '__%s\n putsf ] if
    again ;
}}
````

## Decompiler

Being able to examine compiled code can help in identifying obscure bugs and gaining a deeper understanding of how things work. For this a decompiler is rather useful.

A simple binary dump is pretty easy:

````
:dump (an-)
  [ fetch-next over '%n_%n\n putsf ] times drop ;
````

This can be useful, but the output doesn't help a lot. Consider an example:

    5162 2049 
    5163 4593 
    5164 1 
    5165 4 
    5166 1 
    5167 5173 
    5168 7 
    5169 2049 
    5170 4634 
    5171 2049 
    5172 4352 
    5173 10 

The left column is the offset, the right is the stored value.

It'd be much more useful to map the stored values to instruction names. This is complicated by the fact that Nga allows for packing up to four instructions per cell. To decompile effectively we need a way to unpack them.


````
{{
  :mask #255 and ;
  :next #8 shift ;
  :reorder (abcd-dcba)
    rot push rot push swap pop pop swap ;
---reveal---
  :unpack (n-dcba)
    dup mask swap next
    dup mask swap next
    dup mask swap next
    reorder ;
}}
````

With this I can then proceed to write a quick and dirty function that maps opcodes to a symbolic short name. As with the *Naje* assembler, I use two characters for each (this is sufficient to identify all of the Nga instructions).

The NOP instruction is represented by two periods (I do this for readability purposes). Unrecognized values are rendered as two question marks.

````
:name-instruction
   #0 [ '.. ] case
   #1 [ 'LI ] case
   #2 [ 'DU ] case
   #3 [ 'DR ] case
   #4 [ 'SW ] case
   #5 [ 'PU ] case
   #6 [ 'PO ] case
   #7 [ 'JU ] case
   #8 [ 'CA ] case
   #9 [ 'CC ] case
  #10 [ 'RE ] case
  #11 [ 'EQ ] case
  #12 [ 'NE ] case
  #13 [ 'LT ] case
  #14 [ 'GT ] case
  #15 [ 'FE ] case
  #16 [ 'ST ] case
  #17 [ 'AD ] case
  #18 [ 'SU ] case
  #19 [ 'MU ] case
  #20 [ 'DI ] case
  #21 [ 'AN ] case
  #22 [ 'OR ] case
  #23 [ 'XO ] case
  #24 [ 'SH ] case
  #25 [ 'ZR ] case
  #26 [ 'EN ] case
  drop '?? ;
````

And tying together:

````
:render-packed (n-)
  unpack #4 [ name-instruction puts ] times ;

:disassemble (an-)
  [ fetch-next
    over putn                        (addres)
    dup render-packed ASCII:SPACE putc (inst)
    putn nl                          (opcode)
  ] times drop ;
````
