# A New Listener

The basic listener is very minimalistic. This is the start of
something a little nicer.

# Features

- character breaking input
- suggestions on hitting TAB
- show stack on hitting ESC

# Loading

    retro -i -f alternate-listener.forth
    new-listener

# The Code

~~~
{{
  #1025 'TIB const

  :eol? (c-f)
    { ASCII:CR ASCII:LF ASCII:SPACE } a:contains? ;

  :valid? (s-sf)
    dup s:length n:-zero? ;

  :bs (c-c)
    dup { #8 #127 } a:contains? [ buffer:get buffer:get drop-pair ] if ;

  :hint
    nl TIB d:words-beginning-with nl TIB s:put ;

  :hints (c-c)
    dup ASCII:HT eq? [ buffer:get drop hint ] if ;

  :stack (c-c)
    dup ASCII:ESC eq? [ buffer:get drop nl &dump-stack dip nl TIB s:put ] if ;

  :check (a-)
    [ call ] a:for-each ;

  :c:get (-c) as{ 'liii.... i #1 d }as ;

  :s:get (-s) [ TIB buffer:set
                [ c:get dup buffer:add { &bs &hints &stack } check eol? ] until
                  buffer:start s:chop ] buffer:preserve ;

  :forever (q-)
    repeat &call sip again ;
---reveal---
  :new-listener (-)
    'stty_cbreak unix:system
    [ s:get valid? &interpret &drop choose ] forever ;
}}
~~~
