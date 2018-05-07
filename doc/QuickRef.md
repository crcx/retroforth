# RETRO Quick Reference

Displaying Things
-----------------

Numbers
=======

    #100 n:put


Characters
==========

    $a c:put


Strings
=======

    'hello,_world s:put


Mixed
=====

    #1 #2 #3  '%n_-_%n_=_%n\n s:format s:put
    'Charles' 'Hello_%s\n     s:format s:put


Conditionals
------------

Equality
========


    #2 #3 eq?


Inequality
==========

    #3 #4 -eq?


Greather Than
=============

    #3 #4 gt?


Less Than
=========

    #3 #4 lt?


Greater Than or Equal To
========================

    #3 #4 gteq?


Less Than or Equal To
=====================

    #3 #4 lteq?


String Equality
===============

    'hello  'world  s:eq?


String Inequality
=================

    'hello  'world  s:eq? not


If / Then
=========

    #1 #2 eq? [ 'true  ] [ 'false ] choose
    #1 #2 eq? [ 'true  ] if
    #1 #2 eq? [ 'false ] -if


Multiple Comparisons
====================

    [ @Base
       #8  [ 'octal       ] case
      #10  [ 'decimal     ] case
      #16  [ 'hexadecimal ] case
      drop (default_case)
    ] call


Function Defintions
-------------------

Quotes (Anonymous)
==================

    [  (code)  ]

Quotes can be nested.


Named
=====

    :name  (stack_comment)
      (code) ;

Quotes can be nested inside a named function.


Loops
-----

Unconditional
=============

    repeat (code) again

Conditional loops are generally preferable.


Counted
=======

    (simple,_no_index_on_stack)
    #10 [ $a c:put ] times


Conditional
===========

    #10 [ n:dec dup n:put dup n:-zero? ] while


Math
----

Addition
========

   #100 #200 +


Subtraction
===========

    #400 #32 -


Multiplication
==============

    #98 #12 *


Division
========

    #200 #4 /


Remainder
=========

    #203 #4 mod


Power
=====

Raise 3 to the second power.

    #3 #2 n:pow


Absolute Value
==============

    #-76 n:abs


Minimum and Maximum Value
=========================

    #34 #8 n:min
    #34 #8 n:max
