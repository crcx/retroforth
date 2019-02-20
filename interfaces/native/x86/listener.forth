~~~
{{
  :eol? (c-f)
    [ ASCII:CR eq? ] [ ASCII:LF eq? ] [ ASCII:SPACE eq? ] tri or or ;

  :valid? (s-sf)
    dup s:length n:-zero? ;

  :check-bs (c-c)
    dup [ #8 eq? ] [ #127 eq? ] bi or [ buffer:get buffer:get drop-pair ] if ;

  :c:get (-c) as{ 'liii.... i #1 d }as dup c:put ;

  :s:get (-s) [ #1025 buffer:set
                [ c:get dup buffer:add check-bs eol? ] until
                  buffer:start s:chop ] buffer:preserve ;

  :listen (-)
    test 'RETRO/Native s:put nl
    repeat s:get valid? &interpret &drop choose again ;

  &listen #1 store
}}
~~~
