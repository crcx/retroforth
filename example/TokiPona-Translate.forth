#!/usr/bin/env retro

# Overview

Toki Pona is a very small constructed language. This is
a quick and dirty tool to provide a (very) rough translation
to English.

A couple of resources:

* http://tokipona.org
* http://tokipona.net/tp/janpije/index.php

# The Code

~~~
:translate 
  'a       [ '[emphasis]  ] s:case
  'akesi   [ 'lizard      ] s:case
  'ala     [ 'no          ] s:case
  'alasa   [ 'hunt        ] s:case
  'ale     [ 'all         ] s:case
  'anpa    [ 'low         ] s:case
  'ante    [ 'different   ] s:case
  'anu     [ 'or          ] s:case
  'awen    [ 'keep        ] s:case
  'e       [ '[object]    ] s:case
  'en      [ 'and         ] s:case
  'esun    [ 'shop        ] s:case
  'ijo     [ 'thing       ] s:case
  'ike     [ 'bad         ] s:case
  'ilo     [ 'tool        ] s:case
  'insa    [ 'inside      ] s:case
  'jaki    [ 'dirty       ] s:case
  'jan     [ 'person      ] s:case
  'jelo    [ 'yellow      ] s:case
  'jo      [ 'have        ] s:case
  'kala    [ 'fish        ] s:case
  'kalama  [ 'sound       ] s:case
  'kama    [ 'come        ] s:case
  'kasi    [ 'plant       ] s:case
  'ken     [ 'can         ] s:case
  'kepeken [ 'use         ] s:case
  'kili    [ 'fruit       ] s:case
  'kiwen   [ 'rock        ] s:case
  'ko      [ 'paste       ] s:case
  'kon     [ 'air         ] s:case
  'kule    [ 'color       ] s:case
  'kulupu  [ 'group       ] s:case
  'kute    [ 'hear        ] s:case
  'la      [ '[context]   ] s:case
  'lape    [ 'sleep       ] s:case
  'laso    [ 'green       ] s:case
  'lawa    [ 'head        ] s:case
  'len     [ 'cloth       ] s:case
  'lete    [ 'cold        ] s:case
  'li      [ '[predicate] ] s:case
  'lili    [ 'small       ] s:case
  'linja   [ 'line        ] s:case
  'lipu    [ 'paper       ] s:case
  'loje    [ 'red         ] s:case
  'lon     [ 'at          ] s:case
  'luka    [ 'hand        ] s:case
  'lukin   [ 'see         ] s:case
  'lupa    [ 'hole        ] s:case
  'ma      [ 'land        ] s:case
  'mama    [ 'parent      ] s:case
  'mani    [ 'money       ] s:case
  'meli    [ 'woman       ] s:case
  'mi      [ 'me          ] s:case
  'mije    [ 'man         ] s:case
  'moku    [ 'eat         ] s:case
  'moli    [ 'dead        ] s:case
  'monsi   [ 'back        ] s:case
  'mu      [ '[meow]      ] s:case
  'mun     [ 'moon        ] s:case
  'musi    [ 'play        ] s:case
  'mute    [ 'many        ] s:case
  'nanpa   [ 'number      ] s:case
  'nasa    [ 'strange     ] s:case
  'nasin   [ 'way         ] s:case
  'nena    [ 'mountain    ] s:case
  'ni      [ 'this        ] s:case
  'nimi    [ 'name        ] s:case
  'noka    [ 'foot        ] s:case
  'o       [ '[command]   ] s:case
  'olin    [ 'love        ] s:case
  'ona     [ 'it          ] s:case
  'open    [ 'open        ] s:case
  'pakala  [ 'break       ] s:case
  'pali    [ 'do          ] s:case
  'palisa  [ 'stick       ] s:case
  'pan     [ 'food        ] s:case
  'pana    [ 'give        ] s:case
  'pi      [ 'of          ] s:case
  'pilin   [ 'feel        ] s:case
  'pimeja  [ 'black       ] s:case
  'pini    [ 'end         ] s:case
  'pipi    [ 'bug         ] s:case
  'poka    [ 'near        ] s:case
  'poki    [ 'container   ] s:case
  'pona    [ 'good        ] s:case
  'pu      [ 'book        ] s:case
  'sama    [ 'same        ] s:case
  'seli    [ 'fire        ] s:case
  'selo    [ 'skin        ] s:case
  'seme    [ 'what        ] s:case
  'sewi    [ 'high        ] s:case
  'sijelo  [ 'form        ] s:case
  'sike    [ 'circle      ] s:case
  'sin     [ 'new         ] s:case
  'sina    [ 'you         ] s:case
  'sinpin  [ 'face        ] s:case
  'sitelen [ 'picture     ] s:case
  'sona    [ 'know        ] s:case
  'soweli  [ 'animal      ] s:case
  'suli    [ 'big         ] s:case
  'suno    [ 'sun         ] s:case
  'supa    [ 'table       ] s:case
  'suwi    [ 'sweet       ] s:case
  'tan     [ 'from        ] s:case
  'taso    [ 'but         ] s:case
  'tawa    [ 'to          ] s:case
  'telo    [ 'water       ] s:case
  'tenpo   [ 'time        ] s:case
  'toki    [ 'talk        ] s:case
  'tomo    [ 'house       ] s:case
  'tu      [ 'two         ] s:case
  'unpa    [ 'sex         ] s:case
  'uta     [ 'mouth       ] s:case
  'utala   [ 'fight       ] s:case
  'walo    [ 'while       ] s:case
  'wan     [ 'one         ] s:case
  'waso    [ 'bird        ] s:case
  'wawa    [ 'strong      ] s:case
  'weka    [ 'away        ] s:case
  'wile    [ 'want        ] s:case ;
~~~


~~~
{{
  'Fenced var
  :toggle-fence @Fenced not !Fenced ;
  :fenced? (-f) @Fenced ;
  :handle-line (s-)
    fenced? [ over call ] [ drop ] choose ;
---reveal---
  :unu (sq-)
    swap [ dup   '~~~ s:eq?
           [ drop toggle-fence ]
           [ handle-line       ] choose
         ] file:for-each-line drop ;
}}
~~~

~~~
:clean    [ [ c:letter? ] [ c:whitespace? ] bi or ] s:filter ;
:original dup s:put nl tab ;
:tokens   ASCII:SPACE s:tokenize ;
:convert  [ translate s:put sp ] a:for-each nl ;
#0 sys:argv [ s:to-lower clean original tokens convert ] unu
~~~
