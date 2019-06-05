# Amalgamate

The standard RETRO system is built using the Nga VM[1] and a
copy of the image exported as a C source file[2]. I sometimes
prefer to have these as a single file for easier sharing. This
is a quick little tool to combine them.

Output will be written to stdout.

## References

[1] source/interfaces/retro-unix.c
[2] source/interfaces/retro-image.c

## Code

~~~
{{
  :image:inline
    'source/interfaces/retro-image.c [ s:put nl ] file:for-each-line ;

  :source:line
    dup '#include_"retro-image.c" s:eq?
    [ drop image:inline ] [ s:put nl ] choose ;

---reveal---

  :amalgamate
    'source/interfaces/retro-unix.c [ source:line ] file:for-each-line ;
}}

amalgamate
~~~
