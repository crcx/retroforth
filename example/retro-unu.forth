#!/usr/bin/env retro

# Unu

    unu
    (verb) (-hia) pull out, withdraw, draw out, extract.

Unu is a tool for extracting fenced code blocks from Markdown
documents.

I always found documenting my projects annoying. Eventually I
decided to start mixing the code and commentary using Markdown.
Unu is the tool I use to extract the sources from the original
files. I've found that this makes it easier for me to keep the
commentary up to date, and has lead to better commented code.

## The Code

~~~
{{
  'Fenced var
  :toggle-fence @Fenced not !Fenced ;
  :fenced? (-f) @Fenced ;
  :handle-line (s-)
    fenced? [ over call ] [ drop ] choose ;
---reveal---
  :unu (sq-)
    swap [ dup '~~~ s:eq?
           [ drop toggle-fence ]
           [ handle-line       ] choose
         ] file:for-each-line drop ;
}}

#0 sys:argv [ s:put nl ] unu
~~~

## Commentary

The basic process for this is simple:

- Read a line from a file
- If the line is a fence (~~~) boundary, toggle the fence state
- If not a fence boundary and the fence state is true process the
  line
- Repeat until done

The C implementation displays the lines to *stdout*. For this I
decided that the `unu` word should be a combinator. This makes
it easy to use as the basis for other tools. (See
*example/retro-muri.forth* as a demonstration of this)
