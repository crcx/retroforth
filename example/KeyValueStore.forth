# A Key-Value Data Store

This implements a key-value data store for RETRO. It's not an
optimal solution, but works well enough for smaller data sets.

A store is setup as a linked list with three values:

    0  pointer to previous entry
    1  pointer to key (string)
    2  pointer to value (string) or value (numeric)

This begins with words to access these fields.

~~~
:kv:link  (D-a)       ;
:kv:name  (D-a) n:inc ;
:kv:value (D-a) #2 +  ;
~~~

Next is a word to make a new store. I create a dummy entry
named `(nil)` as the root.

~~~
:kv:new   (s-)   d:create here n:inc , #0 , '(nil) , #0 ,  ;
~~~

Searching the store for an entry is next. This is very inefficient
due to the use of `reorder`. If the performance becomes an issue I
will revisit it later.

~~~
{{
  :match?     (sD-sf)    kv:name fetch over s:eq? ;
  :update     (abc-cbc)  'abc 'cbc reorder ;
  :seek       (nsD-ns)
    repeat fetch 0; &match? sip swap &update if again ;
---reveal---
  :kv:lookup  (sD-a) #0 'abc 'cab reorder seek drop ;
}}
~~~

To add or change an entry, I provide `kv:set`. This is even
more inefficient as it scans the store twice when setting an
existing entry.

This could be improved:

- only scan once
- factor out the update and add entry actions

~~~
:kv:set   (vsD-)
  dup-pair kv:lookup n:-zero?
  [ kv:lookup kv:value store ]
  [ here over [ [ fetch , , , ] dip ] dip store ] choose ;
~~~

The last word is `kv:get`, which returns the contents of the
`kv:value` field.

~~~
:kv:get (sD-v)
  kv:lookup dup 0; drop kv:value fetch ;
~~~

And some tests:

```
'Test kv:new

#1000 'FOO Test kv:set
#1500 'BAR Test kv:set
#4000 'FOOBAR Test kv:set
'FOOBAR Test kv:get n:put nl
#5000 'FOOBAR Test kv:set
'FOOBAR Test kv:get n:put nl
~~~

