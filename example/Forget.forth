# Forget

This is an alternative to `mark`.

As a byproduct you get `hide` which hides a word from later
dictionary search.

~~~
:d:exists? d:lookup n:zero? ;
'abort d:exists? [ 'example/Abort.forth include ] if
's:(evaluate) d:exists? [ 'example/EvaluateString.forth include ] if
~~~

## EOC (end-of-code)

~~~
:d:size (-) #0 [ d:name drop #1 + ] d:for-each ; 
:d:words.last (n-) 
  &Dictionary swap [ fetch dup d:name s:put sp d:link ] times drop ; 
:EOC (-) 'EOC_(end-of-code) s:shout 
  d:size '%n_words_defined s:format s:put nl 
  #4 [ '%n_last_defined_words: s:format s:put sp ] sip d:words.last listen ; 
~~~

## d:hide 

~~~
{{ 
  'This 'Newer [ var ] bi@ 
  :names-match? (a-f) d:name here s:eq? ; 
  :word-exists?continue (s-s) [ dup d:lookup ] assert ; 
  :d:newer-older (s-aa)_newer_older word-exists?continue 
    here s:copy d:last #0 !Newer [ dup names-match? 
    [ @Newer TRUE ] [ dup !Newer d:link fetch FALSE ] choose 
    ] until (this_newer) swap d:link fetch (older) ; 
  :exist?continue (aa-aa) [ dup-pair [ n:-zero? ] bi@ and ] assert ; 
---reveal--- 
  :d:newer (s-a) d:newer-older drop ; 
  :d:hide (s-) d:newer-older exist?continue swap d:link store ; 
}} 
~~~


```
:a ; :b ; :c ; 
'b d:hide 
'#5_d:words.last s:(evaluate) 
```

# Forget

~~~
:d:forget.a (a-) dup n:-zero? [ dup !Heap d:link fetch !Dictionary ] 
  [ drop 'tried_to_d:forget_a_word_that_doesn't_exist. s:shout ] choose ; 
:d:forget (s-) d:lookup d:forget.a ; 
:f:reset (-|float:?-) f:depth [ f:drop ] times ; 
:reset.nf (?-|float:?-) reset f:reset ; 
:d:wipe (-) reset.nf '---wipe--- d:newer d:forget.a ; 
~~~

``` 
:---wipe--- ; 
:a ; :b ; :c ; nl 
'#5_d:words.last s:(evaluate) 
d:wipe nl 
'#5_d:words.last s:(evaluate) nl 
``` 
