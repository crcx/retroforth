This is a quick tool to merge retro-image.c and retro-unix.c into
a single file for easier distribution and building.

~~~
:image:inline
  'source/interfaces/retro-image.c [ s:put nl ] file:for-each-line ;

:source:line
  dup '#include_"retro-image.c" s:eq?
  [ drop image:inline ] [ s:put nl ] choose ;

'source/interfaces/retro-unix.c [ source:line ] file:for-each-line
~~~
