Copyright 2018 jmf
License: WTFPL

This code prints a Mandelbrot set in ASCII art to terminal.

First of all, some variables are declared.
The initial maximum number of iterations is set to 128.

~~~
'x var
'y var
'max_iter var
'iter var

'zoom var
'posx var
'posy var

#0   !posx
#0   !posy
#1   !zoom
#128 !max_iter
~~~

We are using fixed-point numbers with a scaling factor of 10'000.
That means that the float value 1.0 is represented by the number 10'000.
For that, we need a special multiplication definition.

~~~
:f* * #10000 / ;
~~~

Now the calculation of the Mandelbrot set value at a specified point is
defined.

~~~
:mb_value (x_y--v)
#0  !x
#0  !y
#-1 !iter
[ 
@iter #1 + !iter

dup-pair 
#2 @x * @y f* + swap (new_y)
@x @x f* @y @y f* - +  (new_x)
!x !y

@x @x f* @y @y f* + #40000 lteq? 
@iter @max_iter lt? and 
] while
drop drop @iter
;
~~~

The user can change some parameters by entering key press sequences.
The following code checks for those key presses.

~~~
:zoom+? dup $+ eq? [ @zoom #2 * !zoom ] if ;
:zoom-? dup $- eq? [ @zoom #2 / #1 n:max !zoom ] if ;
:up?    dup $w eq? [ @posy #10000 @zoom / - !posy ] if ;
:down?  dup $s eq? [ @posy #10000 @zoom / + !posy ]  if ;
:left?  dup $a eq? [ @posx #10000 @zoom / - !posx ]  if ;
:right? dup $d eq? [ @posx #10000 @zoom / + !posx ]  if ;
:resinc? dup $e eq? [ @max_iter #2 * !max_iter ] if ;
:resdec? dup $q eq? [ @max_iter #2 / #1 n:max !max_iter ]  if ;
~~~

Finally the actual drawing code is written. It renders to 25x80
characters. The width of 80 characters is distributed over a set width
of 3.5 (35000) units, the height of 25 characters is distributed over a
set height of 2 units.
This results in a scaling factor of 438 for the width and 800 for the
height.

~~~
:mb_draw
#25 [
  #80 [
    I #438 * #25000 - @zoom / @posx +
    J #800 * #10000 - @zoom / @posy +
    mb_value
    $A
    swap @max_iter lt? [ drop $. ] if 
    c:put
  ] times<with-index>
  nl
] times<with-index>
;
~~~

The last function is a loop that draws the set, some info and gets key-
presses.

~~~
[
mb_draw
'+/-_to_zoom;_w/a/s/d_to_move s:put nl
'q/e_to_increase_decrease_resolution s:put nl
'zoom_level_ s:put @zoom n:put nl
'iterations_ s:put @max_iter n:put nl 
c:get
zoom+?
zoom-?
up?
down?
left?
right?
resinc?
resdec?
drop
TRUE ] while
~~~
