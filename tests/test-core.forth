In prior releases of RETRO, a test suite covered a large portion
of the core language and libraries. RETRO 12 currently does not
have any sigificant automated tests. This is an attempt to fix
this issue.

As with the previous test suite, this assumes that the core
functionality is mostly working. If your VM or Rx core is broken,
don't expect this to be much help.

First, a way to kill RETRO that'll guarantee a resulting error.
We simply divide by zero.

~~~
:err:die (-) #0 #0 / ;
~~~

Next, the guts of the test suite. I'll define a block for each
word tested, with some minimal syntax. So a test will look
like:

    'wordname Testing
      [ test code ] [ checks, returning a flag ] try
    passed

Multiple tests can be between the `Testing` and the `passed`.
This will count the number of successful tests.

~~~
'Total var
'WordsTested var
'Flag var
'Tests var
'InTestState var

:Testing (s-)
  'Test:__ s:put s:put nl #-1 !Flag #0 !Tests  &WordsTested v:inc reset ;

:passed (-)
  '->_ s:put @Tests n:put '_tests_passed s:put nl
  '----------------------------------- s:put nl ;

:exit-on-fail (-)
  @Flag [ passed '->_1_test_failed s:put nl err:die ] -if ; 

:match (n-)
  eq? @InTestState and !InTestState ;

:try (qq-)
  #-1 !InTestState
  [ call ] dip call
  depth n:-zero? [ @Flag and !Flag ] if
  @Flag @InTestState and !Flag
  exit-on-fail &Tests v:inc &Total v:inc ;

:summary (-)
  @WordsTested n:put '_words_tested s:put nl
  @Total n:put '_tests_passed s:put nl ;
~~~

And now the tests begin. These should follow the order of the
Glossary to make maintenance and checking of completion easier.

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'- Testing
  [ #2 #1 -        ] [ #1 eq? ] try
  [ #2 #4 #3 - -   ] [ #1 eq? ] try
  [ #1 #2 #1 #9 -  ] [ #-8 match #2 match #1 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
', Testing
  [ here #0 , here swap - ] [  #1 eq? ] try
  [ here #12 , fetch      ] [ #12 eq? ] try
  here #1 , #2 , #3 ,
  [ fetch-next swap fetch-next swap fetch ]
  [ #3 eq? swap #2 eq? and swap #1 eq? and ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'; Testing
  [ here &; call here swap - ] [ #1 eq? ] try
  [ here &; call fetch ] [ #10 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'/ Testing
  [ #5 #2 / ] [ #2 eq? ] try
  [ #-5 #2 / ] [ #-2 eq? ] try
  [ #5 #-2 / ] [ #-2 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'[ Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'] Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'{{ Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'}} Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'* Testing
  [  #1  #2 *       ] [ #2 eq?   ] try
  [  #2  #3 *       ] [ #6 eq?   ] try
  [ #-1 #10 *       ] [ #-10 eq? ] try
  [ #-1  #2 * #-1 * ] [ #2 eq?   ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'+ Testing
  [ #1 #2 +  ] [ #3 eq? ] try
  [ #4 #-2 + ] [ #2 eq? ] try
  [ #0 #1 +  ] [ #1 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'0; Testing
  [ #1 0; #2 0; ] [ #2 eq? swap #1 eq? and ] try
  [ #1 0; #0 0; #2 0; ] [ #1 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'again Testing
  [ #3 repeat dup n:dec 0; again ] [ #1 match #2 match #3 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'allot Testing
  [ here #10 allot here swap -  ] [ #10 eq? ] try
  [ here #-10 allot here - ]      [ #10 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'and Testing
  [ #-1 #-1 and ] [ #-1 eq? ] try
  [ #12 #10 and ] [ #8 eq? ] try
  [ #0 #-1 and ] [ #0 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'a:make Testing
  [ { #10 #20 #30 }
    dup a:length #3 eq?
    over #0 a:fetch #10 eq? and
    over #1 a:fetch #20 eq? and
    swap #2 a:fetch #30 eq? and
  ] [ TRUE eq? ] try
  [ { #10 #20 #30 } { #10 #20 #30 } a:eq? ] [ TRUE eq? ] try
  [ { #10 #20 #30 } { #10 #20 #31 } a:-eq? ] [ TRUE eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'Test:ArrayValues var
'Test:ArrayIndex var

:test:string-array? (as-f)
  ASCII:SPACE s:tokenize swap dup-pair [ a:length ] bi@ eq?
  [ !Test:ArrayValues #0 !Test:ArrayIndex ] dip
  swap [ @Test:ArrayValues @Test:ArrayIndex a:fetch s:eq?
         &Test:ArrayIndex v:inc and ] a:for-each ;

'a:make/string-pointers Testing
  [ { 's1 } 's1 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 } 's1_s2 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 } 's1_s2_s3 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 } 's1_s2_s3_s4 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 } 's1_s2_s3_s4_s5 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 } 's1_s2_s3_s4_s5_s6 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 } 's1_s2_s3_s4_s5_s6_s7 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 } 's1_s2_s3_s4_s5_s6_s7_s8 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 's9 } 's1_s2_s3_s4_s5_s6_s7_s8_s9 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 's9 's10 } 's1_s2_s3_s4_s5_s6_s7_s8_s9_s10 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 's9 's10 's11 } 's1_s2_s3_s4_s5_s6_s7_s8_s9_s10_s11 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 's9 's10 's11 's12 } 's1_s2_s3_s4_s5_s6_s7_s8_s9_s10_s11_s12 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 's9 's10 's11 's12 's13 } 's1_s2_s3_s4_s5_s6_s7_s8_s9_s10_s11_s12_s13 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 's9 's10 's11 's12 's13 's14 } 's1_s2_s3_s4_s5_s6_s7_s8_s9_s10_s11_s12_s13_s14 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 's9 's10 's11 's12 's13 's14 's15 } 's1_s2_s3_s4_s5_s6_s7_s8_s9_s10_s11_s12_s13_s14_s15 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 's9 's10 's11 's12 's13 's14 's15 's16 } 's1_s2_s3_s4_s5_s6_s7_s8_s9_s10_s11_s12_s13_s14_s15_s16 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 's9 's10 's11 's12 's13 's14 's15 's16 's17 } 's1_s2_s3_s4_s5_s6_s7_s8_s9_s10_s11_s12_s13_s14_s15_s16_s17 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 's9 's10 's11 's12 's13 's14 's15 's16 's17 's18 } 's1_s2_s3_s4_s5_s6_s7_s8_s9_s10_s11_s12_s13_s14_s15_s16_s17_s18 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 's9 's10 's11 's12 's13 's14 's15 's16 's17 's18 's19 } 's1_s2_s3_s4_s5_s6_s7_s8_s9_s10_s11_s12_s13_s14_s15_s16_s17_s18_s19 test:string-array? ] [ TRUE eq? ] try
  [ { 's1 's2 's3 's4 's5 's6 's7 's8 's9 's10 's11 's12 's13 's14 's15 's16 's17 's18 's19 's20 } 's1_s2_s3_s4_s5_s6_s7_s8_s9_s10_s11_s12_s13_s14_s15_s16_s17_s18_s19_s20 test:string-array? ] [ TRUE eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'a:from-string Testing
  [ 'retro a:from-string
    dup a:length #5 eq?
    over #0 a:fetch $r eq? and
    over #2 a:fetch $t eq? and
    over #4 a:fetch $o eq? and
    swap a:to-string 'retro s:eq? and
  ] [ TRUE eq? ] try
  [ 'retro a:from-string 'retro a:from-string a:eq? ] [ TRUE eq? ] try
  [ 'retro a:from-string 'retros a:from-string a:-eq? ] [ TRUE eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'as{ Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'}as Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:ACK Testing
  [ ASCII:ACK ] [ #6 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:BEL Testing
  [ ASCII:BEL ] [ #7 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:BS Testing
  [ ASCII:BS ] [ #8 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:CAN Testing
  [ ASCII:CAN ] [ #24 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:CR Testing
  [ ASCII:CR ] [ #13 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:DC1 Testing
  [ ASCII:DC1 ] [ #17 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:DC2 Testing
  [ ASCII:DC2 ] [ #18 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:DC3 Testing
  [ ASCII:DC3 ] [ #19 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:DC4 Testing
  [ ASCII:DC4 ] [ #20 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:DEL Testing
  [ ASCII:DEL ] [ #127 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:DLE Testing
  [ ASCII:DLE ] [ #16 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:EM Testing
  [ ASCII:EM ] [ #25 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:ENQ Testing
  [ ASCII:ENQ ] [ #5 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:EOT Testing
  [ ASCII:EOT ] [ #4 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:ESC Testing
  [ ASCII:ESC ] [ #27 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:ETB Testing
  [ ASCII:ETB ] [ #23 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:ETX Testing
  [ ASCII:ETX ] [ #3 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:FF Testing
  [ ASCII:FF ] [ #12 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:FS Testing
  [ ASCII:FS ] [ #28 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:GS Testing
  [ ASCII:GS ] [ #29 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:HT Testing
  [ ASCII:HT ] [ #9 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:LF Testing
  [ ASCII:LF ] [ #10 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:NAK Testing
  [ ASCII:NAK ] [ #21 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:NUL Testing
  [ ASCII:NUL ] [ #0 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:RS Testing
  [ ASCII:RS ] [ #30 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:SI Testing
  [ ASCII:SI ] [ #15 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:SO Testing
  [ ASCII:SO ] [ #14 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:SOH Testing
  [ ASCII:SOH ] [ #1 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:SPACE Testing
  [ ASCII:SPACE ] [ #32 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:STX Testing
  [ ASCII:STX ] [ #2 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:SUB Testing
  [ ASCII:SUB ] [ #26 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:SYN Testing
  [ ASCII:SYN ] [ #22 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:US Testing
  [ ASCII:US ] [ #31 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ASCII:VT Testing
  [ ASCII:VT ] [ #11 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'bi Testing
  [ #1 [ #3 * ] [ #4 + ] bi ] [ #5 match #3 match ] try
  [ #2 [ #3 - ] [ #2 / ] bi ] [ #1 match #-1 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'bi@ Testing
  [ #1 #2 [ #3 *    ] bi@ ] [ #6 match #3 match ] try
  [ #1 #2 [ #3 +    ] bi@ ] [ #5 match #4 match ] try
  [ #1 #2 [ drop #3 ] bi@ ] [ #3 match #3 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'bi* Testing
  [ #1 #2 [ #3 + ] [ #3 * ] bi* ] [ #6 match #4 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'buffer:add Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'buffer:empty Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'buffer:end Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'buffer:get Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'buffer:preserve Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'buffer:set Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'buffer:size Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'buffer:start Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'call Testing
  [ #1 [ ] call #2 ] [ #2 match #1 match ] try
  [ #1 [ #3 ] call #2 ] [ #2 match #3 match #1 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'case Testing
:foo (n-)
  #1 [ #33 ] case
  #2 [ #66 ] case
  drop #44 ;

  [ #0 foo ] [ #44 eq? ] try
  [ #1 foo ] [ #33 eq? ] try
  [ #2 foo ] [ #66 eq? ] try
  [ #3 foo ] [ #44 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:-consonant? Testing
  [ $a ] [ c:-consonant? ] try 
  [ $b ] [ c:-consonant? not ] try 
  [ $c ] [ c:-consonant? not ] try 
  [ $d ] [ c:-consonant? not ] try 
  [ $e ] [ c:-consonant? ] try 
  [ $f ] [ c:-consonant? not ] try 
  [ $g ] [ c:-consonant? not ] try 
  [ $h ] [ c:-consonant? not ] try 
  [ $i ] [ c:-consonant? ] try 
  [ $j ] [ c:-consonant? not ] try 
  [ $k ] [ c:-consonant? not ] try 
  [ $l ] [ c:-consonant? not ] try 
  [ $m ] [ c:-consonant? not ] try 
  [ $n ] [ c:-consonant? not ] try 
  [ $o ] [ c:-consonant? ] try 
  [ $p ] [ c:-consonant? not ] try 
  [ $q ] [ c:-consonant? not ] try 
  [ $r ] [ c:-consonant? not ] try 
  [ $s ] [ c:-consonant? not ] try 
  [ $t ] [ c:-consonant? not ] try 
  [ $u ] [ c:-consonant? ] try 
  [ $v ] [ c:-consonant? not ] try 
  [ $w ] [ c:-consonant? not ] try 
  [ $x ] [ c:-consonant? not ] try 
  [ $y ] [ c:-consonant? not ] try 
  [ $z ] [ c:-consonant? not ] try 
passed
~~~


-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:consonant? Testing
  [ $a ] [ c:consonant? not ] try 
  [ $b ] [ c:consonant? ] try 
  [ $c ] [ c:consonant? ] try 
  [ $d ] [ c:consonant? ] try 
  [ $e ] [ c:consonant? not ] try 
  [ $f ] [ c:consonant? ] try 
  [ $g ] [ c:consonant? ] try 
  [ $h ] [ c:consonant? ] try 
  [ $i ] [ c:consonant? not ] try 
  [ $j ] [ c:consonant? ] try 
  [ $k ] [ c:consonant? ] try 
  [ $l ] [ c:consonant? ] try 
  [ $m ] [ c:consonant? ] try 
  [ $n ] [ c:consonant? ] try 
  [ $o ] [ c:consonant? not ] try 
  [ $p ] [ c:consonant? ] try 
  [ $q ] [ c:consonant? ] try 
  [ $r ] [ c:consonant? ] try 
  [ $s ] [ c:consonant? ] try 
  [ $t ] [ c:consonant? ] try 
  [ $u ] [ c:consonant? not ] try 
  [ $v ] [ c:consonant? ] try 
  [ $w ] [ c:consonant? ] try 
  [ $x ] [ c:consonant? ] try 
  [ $y ] [ c:consonant? ] try 
  [ $z ] [ c:consonant? ] try 
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:-digit? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:digit? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'choose Testing
  [ TRUE [ #1 ] [ #0 ] choose ] [ #1 match ] try
  [ FALSE [ #1 ] [ #0 ] choose ] [ #0 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'class:data Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'class:macro Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'class:primitive Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'class:word Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:letter? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:-lowercase? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:lowercase? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'compile:call Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'compile:jump Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'compile:lit Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'Compiler Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'compile:ret Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'compiling? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'const Testing
  $e 'A const
  [ A ] [ $e eq? ] try
  [ A ] [ $f -eq? ] try
  #100 'B const
  [ B ] [ #100 eq? ] try
  [ B ] [ #-100 -eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'copy Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:toggle-case Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:to-lower Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:to-string Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:to-upper Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:-uppercase? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:uppercase? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'curry Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:-visible? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:visible? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:-vowel? Testing
  [ $a c:-vowel? ] [ FALSE match ] try
  [ $e c:-vowel? ] [ FALSE match ] try
  [ $i c:-vowel? ] [ FALSE match ] try
  [ $o c:-vowel? ] [ FALSE match ] try
  [ $u c:-vowel? ] [ FALSE match ] try
  [ $b c:-vowel? ] [ TRUE match ] try
  [ $c c:-vowel? ] [ TRUE match ] try
  [ $d c:-vowel? ] [ TRUE match ] try
  [ $f c:-vowel? ] [ TRUE match ] try
  [ $g c:-vowel? ] [ TRUE match ] try
  [ $h c:-vowel? ] [ TRUE match ] try
  [ $j c:-vowel? ] [ TRUE match ] try
  [ $k c:-vowel? ] [ TRUE match ] try
  [ $l c:-vowel? ] [ TRUE match ] try
  [ $m c:-vowel? ] [ TRUE match ] try
  [ $n c:-vowel? ] [ TRUE match ] try
  [ $p c:-vowel? ] [ TRUE match ] try
  [ $q c:-vowel? ] [ TRUE match ] try
  [ $r c:-vowel? ] [ TRUE match ] try
  [ $s c:-vowel? ] [ TRUE match ] try
  [ $t c:-vowel? ] [ TRUE match ] try
  [ $v c:-vowel? ] [ TRUE match ] try
  [ $w c:-vowel? ] [ TRUE match ] try
  [ $x c:-vowel? ] [ TRUE match ] try
  [ $y c:-vowel? ] [ TRUE match ] try
  [ $z c:-vowel? ] [ TRUE match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:vowel? Testing
  [ $a c:vowel? ] [ TRUE match ] try
  [ $e c:vowel? ] [ TRUE match ] try
  [ $i c:vowel? ] [ TRUE match ] try
  [ $o c:vowel? ] [ TRUE match ] try
  [ $u c:vowel? ] [ TRUE match ] try
  [ $b c:vowel? ] [ FALSE match ] try
  [ $c c:vowel? ] [ FALSE match ] try
  [ $d c:vowel? ] [ FALSE match ] try
  [ $f c:vowel? ] [ FALSE match ] try
  [ $g c:vowel? ] [ FALSE match ] try
  [ $h c:vowel? ] [ FALSE match ] try
  [ $j c:vowel? ] [ FALSE match ] try
  [ $k c:vowel? ] [ FALSE match ] try
  [ $l c:vowel? ] [ FALSE match ] try
  [ $m c:vowel? ] [ FALSE match ] try
  [ $n c:vowel? ] [ FALSE match ] try
  [ $p c:vowel? ] [ FALSE match ] try
  [ $q c:vowel? ] [ FALSE match ] try
  [ $r c:vowel? ] [ FALSE match ] try
  [ $s c:vowel? ] [ FALSE match ] try
  [ $t c:vowel? ] [ FALSE match ] try
  [ $v c:vowel? ] [ FALSE match ] try
  [ $w c:vowel? ] [ FALSE match ] try
  [ $x c:vowel? ] [ FALSE match ] try
  [ $y c:vowel? ] [ FALSE match ] try
  [ $z c:vowel? ] [ FALSE match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:-whitespace? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'c:whitespace? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd:add-header Testing
  'Test-Add-Header &class:data #0 d:add-header
  [ d:last.name ] [ 'Test-Add-Header s:eq? ] try
  [ d:last.class ] [ &class:data eq? ] try
  [ d:last.xt ] [ #0 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'data Testing
  :Test-Data-Word #123 ; data
  [ 'Test-Data-Word d:lookup d:class fetch ] [ &class:data eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd:class Testing
  [ 'Test-Dictionary-Value d:lookup d:class ] [ #2 + eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd:create Testing
  'Test-Create-XT var
  'Test-Create-Word d:create
  d:last.xt !Test-Create-XT
  [ d:last.name ] [ 'Test-Create-Word s:eq? ] try
  [ d:last.class ] [ &class:data eq? ] try
  [ d:last.xt ] [ @Test-Create-XT eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'depth Testing
  [ depth       ] [ #0 eq? ] try
  [ #1 depth    ] [ #1 eq? reset ] try
  [ #1 #2 depth ] [ #2 eq? reset ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd:for-each Testing
  'Test-Dictionary-Previous var
  'Test-Dictionary-Value    var
  'Test-Dictionary-Seen     var
  #0 !Test-Dictionary-Seen
  [ d:name 'Test-Dictionary-Value s:eq?
    [ TRUE !Test-Dictionary-Seen ] if ] d:for-each
  [ @Test-Dictionary-Seen ] [ TRUE eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'Dictionary Testing
  [ Dictionary ] [ #2 eq? ] try
  [ &Dictionary ] [ #2 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'dip Testing
  [ #1 #2 [ #3 + ] dip ] [ #2 match #4 match ] try
  [ #0 #1 #2 [ [ #3 + ] dip ] dip ] [ #2 match #1 match #3 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd:last Testing
  [ d:last ] [ 'Test-Dictionary-Seen d:lookup eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd:last.class Testing
  [ d:last.class ] [ &class:data eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd:last.name Testing
  [ d:last.name ] [ 'Test-Dictionary-Seen s:eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd:last.xt Testing
  [ d:last.xt ] [ &Test-Dictionary-Seen eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd:link Testing
  [ 'Test-Dictionary-Value d:lookup d:link fetch ]
  [ 'Test-Dictionary-Previous d:lookup eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd:lookup Testing
  [ 'Test-Dictionary-Seen d:lookup ] [ d:last eq? ] try
  [ 'Test-Dictionary-Not-Found d:lookup ] [ n:zero? ] try
  [ 'Test-Dictionary-Seen d:lookup d:xt fetch d:lookup-xt ] [ d:last eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd:name Testing
  [ 'Test-Dictionary-Value d:lookup d:name ]
  [ 'Test-Dictionary-Value s:eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'does Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'drop Testing
  [ #1 #2 drop ] [ #1 eq? ] try
  [ #1 #2 #3 drop ] [ #2 match #1 match ] try
  [ #1 #2 drop drop ] [ #1 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'drop-pair Testing
  [ #1 #2 #3 drop-pair ] [ #1 eq? ] try
  [ #1 #2 drop-pair ] [ depth n:zero? ] try
  [ #1 #2 #3 drop-pair ] [ depth n:-zero? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'dup Testing
  [ #1 dup ] [ #1 match #1 match ] try
  [ #4 #3 dup ] [ #3 match #3 match #4 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'?dup Testing
  [ #1 ?dup ] [ depth #2 match #1 match #1 match ] try
  [ #0 ?dup ] [ depth #1 match #0 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'dup-pair Testing
  [ #2 #3 dup-pair ] [ depth #4 match #3 match #2 match #3 match #2 match ] try
  [ #1 #-1 dup-pair ] [ depth #4 match #-1 match #1 match #-1 match #1 match ] try
  [ #12 #2 #3 dup-pair ] [ depth #5 match #3 match #2 match #3 match #2 match #12 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'd:xt Testing
  [ 'Test-Dictionary-Value d:lookup d:xt fetch ] [ &Test-Dictionary-Value eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'EOM Testing
  [ EOM ] [ #-3 fetch eq? ] try
  [ EOM ] [ here gt? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'-eq? Testing
  [ #1 #2 -eq? ] [ TRUE match ] try
  [ #1 #1 -eq? ] [ FALSE match ] try
  [ #2 #2 -eq? ] [ FALSE match ] try
  [ #2 #1 -eq? ] [ TRUE match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'eq? Testing
  [ #1 #2 eq? ] [ FALSE match ] try
  [ #1 #1 eq? ] [ TRUE match ] try
  [ #2 #2 eq? ] [ TRUE match ] try
  [ #2 #1 eq? ] [ FALSE match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'err:notfound Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'FALSE Testing
  [ FALSE ] [ #0 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'fetch Testing
  'A var
  [ #100 &A store ] [ &A fetch #100 eq? ] try
  [ #200 &A store ] [ &A fetch #200 eq? ] try
  [ #300 &A store ] [ &A fetch #300 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'fetch-next Testing
  'A d:create #1 , #2 , #3 ,
  [ &A fetch-next ] [ #1 match &A #1 + match ] try
  [ &A fetch-next drop fetch-next ] [ #2 match &A #2 + match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'gt? Testing
  [ #1 #2 gt? ] [ FALSE match ] try
  [ #3 #2 gt? ] [ TRUE match ] try
  [ #2 #2 gt? ] [ FALSE match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'gteq? Testing
  [ #1 #2 gteq? ] [ FALSE match ] try
  [ #3 #2 gteq? ] [ TRUE match ] try
  [ #2 #2 gteq? ] [ TRUE match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'Heap Testing
  [ Heap ] [ #3 eq? ] try
  [ &Heap ] [ #3 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'here Testing
  [ here ] [ &Heap fetch eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'i Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'if Testing
  [ #0 TRUE [ #1 ] if ] [ #1 match #0 match ] try
  [ #0 FALSE [ #1 ] if ] [ #0 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'-if Testing
  [ #0 TRUE [ #1 ] -if ] [ #0 match ] try
  [ #0 FALSE [ #1 ] -if ] [ #1 match #0 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'immediate Testing
  :Test-Immediate-Word ; immediate
  [ d:last.class ] [ &class:macro eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'interpret Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'lt? Testing
  [ #1 #2 lt? ] [ TRUE match ] try
  [ #3 #2 lt? ] [ FALSE match ] try
  [ #2 #2 lt? ] [ FALSE match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'lteq? Testing
  [ #1 #2 lteq? ] [ TRUE match ] try
  [ #3 #2 lteq? ] [ FALSE match ] try
  [ #2 #2 lteq? ] [ TRUE match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'mod Testing
  [ #5 #2 mod ] [ #1 eq? ] try
  [ #-5 #2 mod ] [ #-1 eq? ] try
  [ #5 #-2 mod ] [ #1 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'/mod Testing
  [ #5 #2 /mod ] [ #2 match #1 match ] try
  [ #-5 #2 /mod ] [ #-2 match #-1 match ] try
  [ #-5 #-2 /mod ] [ #2 match #-1 match ] try
  [ #5 #-2 /mod ] [ #-2 match #1 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:abs Testing
  [ #1 n:abs ] [ #1 match ] try
  [ #-1 n:abs ] [ #1 match ] try
  [ #0 n:abs ] [ #0 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:between? Testing
  [ #1  #0 #2 n:between? ] [ TRUE match ] try
  [ #-1 #0 #2 n:between? ] [ FALSE match ] try
  [ #1  #10 #20 n:between? ] [ FALSE match ] try
  [ #6  #1 #2000 n:between? ] [ TRUE match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:dec Testing
  [ #1 n:dec ] [ #0 eq? ] try
  [ #10 n:dec ] [ #9 eq? ] try
  [ #100 n:dec ] [ #99 eq? ] try
  [ #-50 n:dec ] [ #-51 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:even? Testing
  [ #2 n:even? ] [ TRUE match ] try
  [ #3 n:even? ] [ FALSE match ] try
  [ #4 n:even? ] [ TRUE match ] try
  [ #5 n:even? ] [ FALSE match ] try
  [ #6 n:even? ] [ TRUE match ] try
  [ #7 n:even? ] [ FALSE match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:inc Testing
  [ #1 n:inc ] [ #2 eq? ] try
  [ #10 n:inc ] [ #11 eq? ] try
  [ #100 n:inc ] [ #101 eq? ] try
  [ #-50 n:inc ] [ #-49 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'nip Testing
  [ #2 #3 nip ] [ #3 eq? ] try
  [ #2 #3 #4 nip ] [ #4 match #2 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'nl Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:limit Testing
  [ #10 #5  #20  n:limit ] [ #10 match ] try
  [ #10 #50 #200 n:limit ] [ #50 match ] try
  [ #10 #12 #20  n:limit ] [ #12 match ] try
  [ #10 #5  #8   n:limit ] [ #8  match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:max Testing
  [ #-6 #3 n:max ] [ #3 match ] try
  [ #6 #-2 n:max ] [ #6 match ] try
  [ #-1 #-6 n:max ] [ #-1 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:MAX Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:min Testing
  [ #-6 #3 n:min ] [ #-6 match ] try
  [ #6 #-2 n:min ] [ #-2 match ] try
  [ #-1 #-6 n:min ] [ #-6 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:MIN Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:negate Testing
  [ #-1 n:negate ] [ #1 match ] try
  [ #0  n:negate ] [ #0 match ] try
  [ #1  n:negate ] [ #-1 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:negative? Testing
  [ #1  n:negative? ] [ FALSE match ] try
  [ #0  n:negative? ] [ FALSE match ] try
  [ #-1 n:negative? ] [ TRUE  match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:odd? Testing
  [ #2 n:odd? ] [ FALSE match ] try
  [ #3 n:odd? ] [ TRUE match ] try
  [ #4 n:odd? ] [ FALSE match ] try
  [ #5 n:odd? ] [ TRUE match ] try
  [ #6 n:odd? ] [ FALSE match ] try
  [ #7 n:odd? ] [ TRUE match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'not Testing
  [ #0 not ] [ TRUE eq? ] try
  [ TRUE not ] [ FALSE eq? ] try
  [ #5 not ] [ #-6 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:positive? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:pow Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:sqrt Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:square Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:strictly-positive? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:to-string Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:-zero? Testing
  [ #1 n:-zero? ] [ #-1 eq? ] try
  [ #0 n:-zero? ] [ #0 eq? ] try
  [ #-1 n:-zero? ] [ #-1 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:arithmetic Testing
  [ #1 #2 n:add ] [ #3 eq? ] try
  [ #3 #1 n:sub ] [ #2 eq? ] try
  [ #2 #3 n:mul ] [ #6 eq? ] try
  [ #7 #3 n:divmod ] [ #2 match #1 match ] try
  [ #7 #3 n:div ] [ #2 eq? ] try
  [ #7 #3 n:mod ] [ #1 eq? ] try
  [ &n:add &+ eq? ] [ TRUE eq? ] try
  [ &n:sub &- eq? ] [ TRUE eq? ] try
  [ &n:mul &* eq? ] [ TRUE eq? ] try
  [ &n:divmod &/mod eq? ] [ TRUE eq? ] try
  [ &n:div &/ eq? ] [ TRUE eq? ] try
  [ &n:mod &mod eq? ] [ TRUE eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'n:zero? Testing
  [ #1 n:zero? ] [ #0 eq? ] try
  [ #0 n:zero? ] [ #-1 eq? ] try
  [ #-1 n:zero? ] [ #0 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'or Testing
  [ #0 #0 or ] [ #0 eq? ] try
  [ #12 #10 or ] [ #14 eq? ] try
  [ #0 TRUE or ] [ TRUE eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'over Testing
  [ #1 #2 over ] [ #1 match #2 match #1 match ] try
  [ #1 #2 #3 over ] [ #2 match #3 match #2 match #1 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'pop Testing
  [ #1 dup push #2 pop ] [ #1 match #2 match #1 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'prefix:` Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'prefix:: Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'prefix:! Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'prefix:' Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'prefix:( Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'prefix:@ Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'prefix:$ Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'prefix:& Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'prefix:# Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'push Testing
  [ #1 dup push #2 pop ] [ #1 match #2 match #1 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'putc Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'putn Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'puts Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'r Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'reclass Testing
  :Test-Reclass-Word ; &class:data reclass
  [ d:last.class ] [ &class:data eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'reorder Testing
  [ #1 #2 #3 #4 'abcd 'dcba reorder ]
  [ #1 match #2 match #3 match #4 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'repeat Testing
  [ #3 repeat dup n:dec 0; again ] [ #1 match #2 match #3 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'reset Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'---reveal--- Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'RewriteUnderscores Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'rot Testing
  [ #1 #2 #3 rot ] [ #1 match #3 match #2 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's, Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:append Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:case Testing
:foo (n-)
  'cat [ #33 ] s:case
  'egg [ #66 ] s:case
  drop #44 ;

  [ 'boo foo ] [ #44 eq? ] try
  [ 'cat foo ] [ #33 eq? ] try
  [ 'egg foo ] [ #66 eq? ] try
  [ 'forth foo ] [ #44 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:chop Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:contains-char? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:contains-string? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'ScopeList Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:empty Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'set:contains? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'set:contains-string? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'set:dup Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'set:filter Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'set:counted-results Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'set:from-string Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'set:for-each Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'set:length Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'set:map Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'set:nth Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'set:reverse Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:eq? Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:filter Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:for-each Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:hash Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'shift Testing
  [ #455 #-3 shift ] [ #3640 eq? ] try
  [ #3640 #3 shift ] [ #455 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'shift-left Testing
  [ #455 #3 shift-left ] [ #3640 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'shift-right Testing
  [ #3640 #3 shift-right ] [ #455 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:index-of Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'sip Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:keep Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:left Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:length Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:map Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'sp Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:prepend Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:reverse Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:right Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:skip Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:split Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:substr Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:temp Testing
  [ STRINGS ] [ EOM @TempStrings @TempStringMax STRING-TERMINATOR-CELLS + * - eq? ] try
  [ s:empty s:length ] [ #0 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:to-lower Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:to-number Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'store Testing
  'A var
  [ #100 &A store ] [ &A fetch #100 eq? ] try
  [ #200 &A store ] [ &A fetch #200 eq? ] try
  [ #300 &A store ] [ &A fetch #300 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'store-next Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:to-upper Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:trim Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:trim-left Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:trim-right Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'STRINGS Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'swap Testing
  [ #1 #2 #3 swap ] [ #2 match #3 match #1 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
's:format Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'tab Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'TempStringMax Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'TempStrings Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'times Testing
  [ #5 [ #1 ] times ] [ #1 match #1 match #1 match #1 match #1 match ] try
  [ #3 [ #1 ] times ] [ #1 match #1 match #1 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'tri Testing
  [ #30 [ #1 + ] [ #2 * ] [ #30 - ] tri ] [ #0 match #60 match #31 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'tri@ Testing
  [ #1 #2 [ #30 * ] tri@ ] [ #60 match #30 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'tri* Testing
  [ #1 #2 #3 [ #1 - ] [ #2 - ] [ #3 - ] tri* ] [ #0 match #0 match #0 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'TRUE Testing
  [ TRUE ] [ #-1 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'tuck Testing
  [ #1 #2 tuck ] [ #2 match #1 match #2 match ] try
  [ #3 #1 #2 tuck ] [ #2 match #1 match #2 match #3 match ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'until Testing
  'Test-Until-Count var
  [ #0 !Test-Until-Count
    [ &Test-Until-Count v:inc @Test-Until-Count #3 eq? ] until ]
  [ @Test-Until-Count #3 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'var Testing
  'A var
  'B var
  [ #10 &A store ] [ &A fetch #10 eq? ] try
  [ #20 &B store ] [ &B fetch #20 eq? ] try
  [ #30 &A store ] [ &A fetch #30 eq? ] try
  [ #50 &B store ] [ &B fetch #50 eq? ] try
  [ #100 A store ] [ A fetch #100 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'var-n Testing
  #30 'A var-n
  #40 'B var-n
  [ ] [ &A fetch #30 eq? ] try
  [ ] [ &B fetch #40 eq? ] try
  [ #10 &A store ] [ &A fetch #10 eq? ] try
  [ #20 &B store ] [ &B fetch #20 eq? ] try
  [ #30 &A store ] [ &A fetch #30 eq? ] try
  [ #50 &B store ] [ &B fetch #50 eq? ] try
  [ #100 A store ] [ A fetch #100 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'v:dec Testing
  #199 'A var-n
  [ &A v:dec ] [ &A fetch #198 eq? ] try
  [ &A v:dec ] [ &A fetch #197 eq? ] try
  [ &A v:dec ] [ &A fetch #196 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'v:dec-by Testing
  #199 'A var-n
  [ #2 &A v:dec-by ] [ &A fetch #197 eq? ] try
  [ #3 &A v:dec-by ] [ &A fetch #194 eq? ] try
  [ #5 &A v:dec-by ] [ &A fetch #189 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'Version Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'v:inc Testing
  'A var
  [ #0 &A store
    &A v:inc    &A v:inc    &A v:inc ] [ &A fetch #3 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'v:inc-by Testing
  'TestA var
  [ #10 &TestA v:inc-by ] [ @TestA #10 eq? ] try
  [ #10 &TestA v:inc-by ] [ @TestA #20 eq? ] try
  [ #10 &TestA v:inc-by ] [ @TestA #30 eq? ] try
  [ #-20 &TestA v:inc-by ] [ @TestA #10 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'v:limit Testing
  'A var
  [ #100 !A &A #-10 #10 v:limit ] [ @A #10 eq? ] try
  [ #-100 !A &A #-10 #10 v:limit ] [ @A #-10 eq? ] try
  [ #6 !A &A #-10 #10 v:limit ] [ @A #6 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'v:off Testing
  'A var
  [        &A v:off ] [ @A n:zero? ] try
  [ #1  !A &A v:off ] [ @A n:zero? ] try
  [ #-1 !A &A v:off ] [ @A n:zero? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'v:on Testing
  'A var
  [        &A v:on ] [ @A #-1 eq? ] try
  [ #0  !A &A v:on ] [ @A #-1 eq? ] try
  [ #-2 !A &A v:on ] [ @A #-1 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'v:preserve Testing
  'A var
  [ #100 &A store &A [ #40 &A store ] v:preserve ] [ &A fetch #100 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'v:update Testing
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'while Testing
  'Test-While-Count var
  [ #0 !Test-While-Count
    [ &Test-While-Count v:inc @Test-While-Count #3 lt? ] while ]
  [ @Test-While-Count #3 eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
'xor Testing
  [ #-1 #-1 xor ] [ #0 eq? ] try
  [ #12 #10 xor ] [ #6 eq? ] try
  [ #0 TRUE xor ] [ TRUE eq? ] try
passed
~~~

-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

~~~
summary
~~~
