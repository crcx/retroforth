# Dictionary: Word Used In ...

Given a word `w0` , I wish to know all words that uses `w0`.
To that end, first I need to know whether a word `w1` uses
`w0` or not.

There are two candidate cell patterns for `w0` usage in `w1`.
These are:

`'lica.... i` or `LIteral CAll 0 0` in nga.
`'lica.... i` in nga is #2049.

~~~
#2049 'LICA.... const (LIteral_CAll_0_0_instruction
:lica....? (n-f) LICA.... eq? ;
~~~

`'liju.... i` or `LIteral JUmp 0 0` in nga.
`'liju.... i` in nga is #1793.

~~~
#1793 'LIJU.... const (LIteral_JUmp_0_0_instruction
:liju....? (n-f) LIJU.... eq? ;
~~~

The `pattern?` word will check for both.

~~~
:pattern? [ lica....? ] [ liju....? ] bi or ;
~~~

If pattern is followed by `w0`'s execution token `xt0`, 
then `w1` uses `w0`.

The pattern of two cells to be sought is

  LICA....  (or LIJU....)
  xt0

The name of `w1` may be found by `'a1 d:lookup d:name`
in which `a1` is `w1`'s address.

Variables `Used` contains `a0` and `In` contains `a1`.
Make sure the word's class is `class:word` and not something els,
like `class:primitive`, `class:data`, etc.
 
~~~
'Used var (a0
'In   var (a1
:class:word? (a-f) d:class fetch &class:word eq? ;
:used (a-) dup !Used ;
:in   (a-) dup !In   ;
~~~

Set the code range to search for the pattern.

~~~
'Start var
'End   var
~~~

The start is the execution token.

~~~
:start (-) @In d:xt !Start ; 
~~~

The end is two cells before the first newer entry to the
dictionary.

If that cell is not `lica...`, then the next cell does not 
hold an execution token.

The first newer entry may be found by `d:new fetch` .

~~~
:d:new-old (a-aa)_returns_(This_0)_if_This_is_d:last
  d:last dup-pair eq?     (this_last_flag
  [ drop #0 ] if;         (exit_if_this_is_last (this_last
  [ dup-pair d:link fetch (this_newer_this_new
    eq? [ TRUE ] [ d:link fetch FALSE ] choose (this_newer_flag
  ] until (this_newer
  swap d:link fetch ;     (newer_old

:d:new (a-a) d:new-old drop ;

:end (-) @In d:new dup n:zero?
  [ here #2 - ] [ dup #2 - n:max ] choose !End ;
~~~

Check if a pair of cells indicates that the word is used.

~~~
:pair? (a-f) dup fetch pattern? 
  [ #1 + fetch @Used d:xt fetch eq? ] [ drop FALSE ] choose ;
~~~

~~~
'Address var (head_address_of_the_pair_being_checked
'More    var (TRUE_=_there_are_remaining_pairs_to_check
'Found   var (TRUE_=_@In_used_@Used
:more    (-) @Address @End lt? [ TRUE ] [ FALSE ] choose !More ;
:prepare (-) start end  @Start !Address  FALSE !Found  more ;

:uses? (-f)_does_@In_use_@Used_?
  prepare [ 
    (pair-found @Address pair? [ &Found v:on ] if )
    (more-to-check more &Address v:inc )
    (condition-to-loop @Found not @More and ) ] while @Found ;
~~~

```
:t (aa-) used in start end ;
:a 'a s:put ;
:b a 'b s:put ;
:c b 'c s:put ;
'b d:lookup in 'a d:lookup used
uses? n:put nl
```

Final product.

~~~
:used-in (a-)
  used 
  [ dup class:word?
    [ in uses? [ @In d:name s:put sp ] if ] [ drop ] choose 
  ] d:for-each drop ;
:d:used-in (s-) d:lookup used-in ;
~~~

```
'a d:lookup used-in
```

Hide unnecessary words.

~~~
'LICA....  d:lookup d:link fetch
'd:used-in d:lookup d:link store 
~~~~
