# VT100

This is a namespace for working with VT100/ANSI escape sequences. It's
intended to help make it easier to create TUI (textual user interface)
based applications.


## Base Functionality

~~~
:VT100:escape (-)
  ASCII:ESC putc ;
~~~

Everything starts with an ESCAPE character.

## Terminal Setup

~~~
:VT100:reset (-)
  VT100:escape $c putc ;
~~~

~~~
:VT100:enable-line-wrap (-)
  VT100:escape '[7h puts ;
~~~

~~~
:VT100:disable-line-wrap (-)
  VT100:escape '[7l puts ;
~~~


## Fonts

~~~
:VT100:default-font (-)
  VT100:escape $( putc ;
~~~

~~~
:VT100:alternate-font (-)
  VT100:escape $) putc ;
~~~


Cursor Control

~~~
:VT100:cursor:home (-)
  VT100:escape '[0;0H puts ;
~~~

~~~
:VT100:cursor:move (rc-)
  VT100:escape
  swap '[%n;%nH s:with-format puts ;
~~~

~~~
:VT100:cursor:down (n-)
  VT100:escape '[%nB s:with-format puts ;
~~~

~~~
:VT100:cursor:up (n-)
  VT100:escape '[%nA s:with-format puts ;
~~~

~~~
:VT100:cursor:forward (n-)
  VT100:escape '[%nC s:with-format puts ;
~~~

~~~
:VT100:cursor:back (n-)
  VT100:escape '[%nD s:with-format puts ;
~~~

~~~
:VT100:cursor:save (-)
  VT100:escape '[s puts ;
~~~

~~~
:VT100:cursor:restore (-)
  VT100:escape '[u puts ;
~~~

~~~
:VT100:cursor:save-attributes (-)
  VT100:escape '[7 puts ;
~~~

~~~
:VT100:cursor:restore-attributes (-)
  VT100:escape '[8 puts ;
~~~


## Attributes

~~~
:VT100:color (s-n)
  'black    [ #30 ] s:case
  'red      [ #31 ] s:case
  'green    [ #32 ] s:case
  'yellow   [ #33 ] s:case
  'blue     [ #34 ] s:case
  'magenta  [ #35 ] s:case
  'cyan     [ #36 ] s:case
  drop #37 ;
~~~

~~~
:VT100:foreground (n-n) ;
~~~

~~~
:VT100:background (n-n) #10 + ;
~~~

~~~
:VT100:set (n-)
  VT100:escape '[%nm s:with-format puts ;
~~~

~~~
:VT100:clear (-)
  VT100:escape '[2J puts ;
~~~

## Example:

    VT100:clear
    #10 #15 VT100:cursor:move
    'red VT100:color VT100:foreground VT100:set
    'Hello_World! puts nl
    #11 #15 VT100:cursor:move
    'cyan VT100:color VT100:foreground VT100:set
    'Press_a_key_to_exit puts nl
    getc drop
    VT100:reset
