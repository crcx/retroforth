#!/usr/bin/env retro

# Export as HTML

This tool processes the code in Unu code blocks, generating HTML
output. It assumes that a CSS file will be used to style the
results. A sample CSS is included at the end.

# Code

I include my `unu` file processor here as it makes it easy to
process the relevant lines from the source file.

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
~~~

Some characters need to be escaped. The `:put-html` words will
be used to handle these.

~~~
:c:put-html
  $< [ '&lt;  s:put ] case
  $> [ '&gt;  s:put ] case
  $& [ '&amp; s:put ] case
  c:put ;
:s:put-html [ c:put-html ] s:for-each ;
~~~

Formatting is pretty straightforward. I have a `span` word to
generate the HTML for the token, and a `format` word to identify
the token type and use `span`.

~~~
:span '<span_class=' s:put s:put ''> s:put s:put-html '</span> s:put ;

:format (s-)
  (ignore_empty_tokens)
  dup s:length n:zero? [ drop ] if;

  (tokens_with_prefixes)
  dup fetch
  $: [ 'colon     span ] case
  $( [ 'comment   span ] case
  $' [ 'string    span ] case
  $# [ 'number    span ] case
  $& [ 'pointer   span ] case
  $$ [ 'character span ] case
  $` [ 'inst      span ] case
  $| [ 'defer     span ] case
  $@ [ 'fetch     span ] case
  $! [ 'store     span ] case

  (immediate_words)
  drop dup
  ';      [ 'immediate span ] s:case
  '0;     [ 'immediate span ] s:case
  'hook   [ 'immediate span ] s:case
  'as{    [ 'immediate span ] s:case
  '}as    [ 'immediate span ] s:case
  '{      [ 'immediate span ] s:case
  '}      [ 'immediate span ] s:case
  'pop    [ 'immediate span ] s:case
  'push   [ 'immediate span ] s:case
  'again  [ 'immediate span ] s:case
  'repeat [ 'immediate span ] s:case
  '[      [ 'immediate span ] s:case
  ']      [ 'immediate span ] s:case
  ')      [ 'comment   span ] s:case

  (primitive_and_normal_words)
  d:lookup d:class fetch &class:primitive eq?
  [ 'primitive span ] [ s:put '&nbsp; s:put ] choose ;
~~~

The remaining bits are just to generate the header, and then
process each line through the `format` word.

~~~
'<!DOCTYPE_html> s:put nl
'<link_rel='stylesheet'_href='/retro-export.css'> s:put nl

#0 sys:argv
  [ ASCII:SPACE s:tokenize [ format '&nbsp; s:put ] array:for-each
    '<br> s:put nl ] unu
~~~

# CSS

* {
  background: #111;
  color: #44dd44;
  font-family: monospace;
}

.colon {
  color: red;
}
.comment {
  color: #888;
}
.string {
  color: yellow;
}
.number {
  color: cyan;
}
.pointer {
  color: mediumorchid;
}
.fetch {
  color: orchid;
}
.store {
  color: orchid;
}
.character {
  color: brown;
}
.inst {
  color: tomato;
}
.defer {
  color: #888;
}

.immediate {
  color: orange;
}

.primitive {
  color: greenyellow;
}
