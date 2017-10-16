#!/usr/bin/env rre

~~~
{{
  'FID var
  'FSize var
  'Action var
  'Buffer var
  :-eof? (-f) @FID file:tell @FSize lt? ;
---reveal---
  :file:read-line (f-s)
    !FID
    [ s:empty dup !Buffer buffer:set
      [ @FID file:read dup buffer:add
        [ ASCII:CR eq? ] [ ASCII:LF eq? ] [ ASCII:NUL eq? ] tri or or ] until
        buffer:get drop ] buffer:preserve
    @Buffer ;

  :file:for-each-line (sq-)
    !Action
    file:R file:open !FID
    @FID file:size !FSize
    [ @FID file:read-line @Action call -eof? ] while
    @FID file:close ;
}}
~~~

~~~
#0 sys:argv [ puts nl ] file:for-each-line
~~~
