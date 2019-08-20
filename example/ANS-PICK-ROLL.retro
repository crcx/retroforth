PICK and ROLL are problematic in that they require the ability
to address the stack as if it were an array. The implementations
here are not efficient as RETRO's stacks are *not* addressable.

These will never be added to the standard image, but are provided
here as an aid in porting ANS FORTH code or for those curious as
to how such things could be added.

# PICK

6.2.2030 PICK 
CORE EXT

	( xu ... x1 x0 u -- xu ... x1 x0 xu )

Remove u. Copy the xu to the top of the stack. An ambiguous
condition exists if there are less than u+2 items on the stack
before PICK is executed.


~~~
{{
  :save-stack (...-a)
    here [ depth &, times ] dip ;
  :fetch-prior (a-n[a-1])
    dup fetch swap n:dec ;
  :restore-stack (a-...)
    here swap - here n:dec swap [ fetch-prior ] times drop ;
---reveal---
  :PICK (...n-...m)
    &Heap [ [ save-stack ] dip
            over + fetch [ restore-stack ] dip ] v:preserve ;
}}
~~~

# ROLL

6.2.2150 ROLL 
CORE EXT 

	( xu xu-1 ... x0 u -- xu-1 ... x0 xu )

Remove u. Rotate u+1 items on the top of the stack. An ambiguous
condition exists if there are less than u+2 items on the stack
before ROLL is executed.

~~~
{{
  :save-values (...n-a)
    [ , ] times here ;
  :restore-values (a-...)
    here - here swap [ fetch-next swap ] times drop ;
---reveal---
  :ROLL (...n-...m)
    &Heap [ save-values ] v:preserve swap [ restore-values ] dip ;
}}
~~~
