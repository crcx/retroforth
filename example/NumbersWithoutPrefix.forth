Numbers without the # Prefix

RETRO requires a # prefix to identify numbers. But since new
prefixes can be defined, it's possible to define prefixes for
the number starting with 0-9.

This is actually easy (at least for positive numbers). Just
define a prefix handler that prepends the initial value back
on the string (since the string sans the prefix is passed to
the handler) and pass it to `s:to-number` and `class:data`.

~~~
{{
  :process-with-prefix (s-n)
    s:prepend s:to-number class:data ;
---reveal---
  :prefix:1 (s-n) '1 process-with-prefix ; immediate
  :prefix:2 (s-n) '2 process-with-prefix ; immediate
  :prefix:3 (s-n) '3 process-with-prefix ; immediate
  :prefix:4 (s-n) '4 process-with-prefix ; immediate
  :prefix:5 (s-n) '5 process-with-prefix ; immediate
  :prefix:6 (s-n) '6 process-with-prefix ; immediate
  :prefix:7 (s-n) '7 process-with-prefix ; immediate
  :prefix:8 (s-n) '8 process-with-prefix ; immediate
  :prefix:9 (s-n) '9 process-with-prefix ; immediate
~~~

`0` is a special case. Since RETRO has `0;` as a control
flow word, the `0` prefix would prevent using it. So the
handler for this checks to see if the part following the
prefix is a `;`. If so, it'll fall back to `0;`, otherwise
it treats the token as a number.

~~~
  :prefix:0 (s-n)
    dup '; s:eq?
    [ drop &0; call ]
    [ '0 process-with-prefix ] choose ; immediate
}}
~~~

For single digits, define each digit as a word. (Prefixes
are not processed for tokens with nothing other than the
prefix character, so this takes care of the issue).

~~~
:0 (-n) #0 ;
:1 (-n) #1 ;
:2 (-n) #2 ;
:3 (-n) #3 ;
:4 (-n) #4 ;
:5 (-n) #5 ;
:6 (-n) #6 ;
:7 (-n) #7 ;
:8 (-n) #8 ;
:9 (-n) #9 ;
~~~

Handling negative numbers is a bigger headache though. By
convention, RETRO uses - to imply "not" (as in `-eq?`). So
to handle the `-` prefix for numbers, it needs to fall back
to a dictionary search if the token isn't actually a number.

~~~
{{
  :numeric? (s-sf)
    dup fetch c:digit? ;
  :in-dictionary? (s-df)
    '- s:prepend d:lookup dup n:zero? ;
  :report-error (d-)
    drop err:notfound ;
  :call-with-class (d-)
    [ d:xt fetch ] [ d:class fetch ] bi call ;
---reveal---
  :prefix:- (s-)
    numeric?
    [ s:to-number n:negate class:data ]
    [ in-dictionary? &report-error &call-with-class choose ] choose ; immediate
}}
~~~

