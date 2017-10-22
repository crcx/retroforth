# RETRO Cross Reference

Definitions
-----------
RETRO    :name ;
ANS      : name ;


Numbers
-------
RETRO    #100  #-12
ANS      100   -12


Characters
----------
RETRO    $A      $D
ANS      CHAR A  CHAR D

RETRO    :foo $A ;
ANS      : foo [CHAR] A ;


Comments
--------
RETRO    (This_is_a_comment)
ANS      ( This is a comment )


Pointers
--------
RETRO    &Compiler
ANS      ' Compiler
RETRO    :foo &Heap ;
ANS      : foo ['] Heap ;


Conditionals
------------
RETRO    (flag) [ 'TRUE ] if
ANS      ( flag ) IF s" TRUE" THEN

RETRO    (flag) [ 'FALSE ] -if
ANS      ( flag ) NOT IF s" FALSE" THEN

RETRO    (flag) [ 'TRUE ] [ 'FALSE ] choose
ANS      ( flag ) IF s" TRUE" ELSE s" FALSE" THEN

RETRO conditionals can be used outside of definitions, ANS ones can not.
