# Assertion

Copyright [c] 2011, Marc Simpson
Updated for RETRO 12 by Charles Childers, 2018

## Description

This vocabulary provides support for testing code in a clean, predicatable manner.

Assertion predicates first check the stack for underflow; if the stack is deep enough, their embedded predicates are applied; if not, the assertion fails.

The result of each assertion - including the underflow check - is ANDed with the assertionFlag which can then be tested after the containing thread has finished executing; this is handled by the .assertion word class.

For custom behaviour, revector preCond and/or postCond; by default the pre-condition is an effective nop while the post-condition simply prints 'Success' or 'Failure'.

Given that each assertion predicate mutates assertionFlag, use of the word class `class:assertion` is encouraged; this resets the `assertionFlag` before execution and push its final value to the stack before calling postCond when the thread exits.

NOTE: For simplicity of implementation, failure within a word of class `class:assertion` will not result in immediate termination; instead, the false value of `assertionFlag` is left to propagate.


## Code & Commentary

~~~
'assertionFlag var

:assert  (f-)
  &assertionFlag [ and ] v:update-using ;

:deep?   (n-f)
  n:inc depth lteq? dup assert ;

:assert:eq    #2 deep? [ eq?      assert ] if ;
:assert:false #1 deep? [ n:zero?  assert ] if ;
:assert:true  #1 deep? [ n:-zero? assert ] if ;
:assert:gt    #2 deep? [ gt?      assert ] if ;
:assert:gteq  #2 deep? [ gteq?    assert ] if ;
:assert:lt    #2 deep? [ lt?      assert ] if ;
:assert:lteq  #2 deep? [ lteq?    assert ] if ;

:putAssertion (f-)
  nl [ 'Success ] [ reset 'Failure ] choose s:put ;
~~~

These are from Hooks.forth:

~~~
:hook (-)  #1793 , here n:inc , ; immediate
:set-hook (aa-) n:inc store ;
:unhook (a-) n:inc dup n:inc swap store ;
~~~

~~~
:preCond  (-)  hook ;
:postCond (f-) hook putAssertion ;

{{
  :buildUp preCond  &assertionFlag v:on ;
  :tearDown @assertionFlag  &assertionFlag v:on  postCond ;
---reveal---
  :class:assertion (a-)
    &buildUp class:word class:word &tearDown class:word ;
}}

:assertion (-) &class:assertion reclass ;
~~~


~~~
  :a1 (xyz-) #30 assert:eq #20 assert:eq #10 assert:eq ; assertion
  :a2 (xy-) assert:true assert:false ; assertion
  :a3 (x-) #5 assert:gt ; assertion

  :go
    nl '---------- s:put
    #10 #20 #30 a1
    #30 #20 #10 a1
    nl '---------- s:put
    #10 a1
    nl '---------- s:put
    a1
    nl '---------- s:put
    #-1 #0 a2
    #0 #-1 a2
    #-1 #-1 a2
    #-1 a2
    nl '---------- s:put
    #3 a3
    #4 a3
    #5 a3
    a3
    nl '---------- s:put
  ;

  go
~~~


## Functions

+-----------------+-----+-----------------------+
| Name            |Stack| Usage                 |
+=================+=====+=======================+
| assertionFlag   | -a  | Variable. This holds a|
|                 |     | true (-1) or false (0)|
|                 |     | value indicating      |
|                 |     | whether the current   |
|                 |     | set of assertions have|
|                 |     | passed or failed. The |
|                 |     | **class:assertion**   |
|                 |     | class will set this to|
|                 |     | true automatically.   |
+-----------------+-----+-----------------------+
| assert          | f-  | Updates the           |
|                 |     | **assertionFlag**     |
|                 |     | using bitwise AND     |
+-----------------+-----+-----------------------+
| deep?           | n-f | Checks to see if there|
|                 |     | are at least *n* items|
|                 |     | on the stack.         |
|                 |     | Returns true if there |
|                 |     | are, false otherwise. |
+-----------------+-----+-----------------------+
| assert:eq       | nn- | Check to see if two   |
|                 |     | values are equal      |
+-----------------+-----+-----------------------+
| assert:false    | f-  | Check to see if flag  |
|                 |     | is false (0)          |
+-----------------+-----+-----------------------+
| assert:true     | f-  | Check to see if flag  |
|                 |     | is true (non-zero)    |
+-----------------+-----+-----------------------+
| assert:gt       | nn- | Check to see if n1 is |
|                 |     | greather than n2      |
+-----------------+-----+-----------------------+
| assert:gteq     | nn- | Check to see if n1 is |
|                 |     | greater than or equal |
|                 |     | to n2                 |
+-----------------+-----+-----------------------+
| assert:lt       | nn- | Check to see if n1 is |
|                 |     | less than n2          |
+-----------------+-----+-----------------------+
| assert:lteq     | nn- | Check to see if n1 is |
|                 |     | less than or equal to |
|                 |     | n2                    |
+-----------------+-----+-----------------------+
| putAssertion    | f-  | Display 'Success' or  |
|                 |     | 'Failure' based on    |
|                 |     | flag                  |
+-----------------+-----+-----------------------+
| preCond         | -   | Hook, does nothing by |
|                 |     | default               |
+-----------------+-----+-----------------------+
| postCond        | f-  | Hook, displays either |
|                 |     | 'Success' or 'Failure'|
|                 |     | by default            |
+-----------------+-----+-----------------------+
| class:assertion | a-  | Class handler for     |
|                 |     | assertions            |
+-----------------+-----+-----------------------+
| assertion       | -   | Change the class of a |
|                 |     | function to           | 
|                 |     | class:asssertion      |
+-----------------+-----+-----------------------+

