The Tower of Hanoi (also called the Tower of Brahma or Lucas' Tower
and sometimes  pluralized) is a mathematical game or puzzle. It
consists of three rods and a number of disks of different sizes,
which can slide onto any rod. The puzzle starts with the disks in 
a neat stack in ascending order of size on one rod, the smallest
at the top, thus making a conical shape.

The objective of the puzzle is to move the entire stack to another
rod, obeying the following simple rules:

- Only one disk can be moved at a time.
- Each move consists of taking the upper disk from one of the
  stacks and placing it on top of another stack.
- No disk may be placed on top of a smaller disk.

With 3 disks, the puzzle can be solved in 7 moves. The minimal
number of moves required to solve a Tower of Hanoi puzzle is
2^n-1, where n is the number of disks.

Taken from https://en.m.wikipedia.org/wiki/Tower_of_Hanoi

I am tracking state in a few variables, so I define them first.

~~~
'Num  var
'From var
'To   var
'Via  var
~~~

Next, a quick word to setup all the variables.

~~~
:set-vars !Via !To !From !Num ;
~~~

Then, implementing the recursive alogrithim from the Wikipedia article:

~~~
:hanoi (num,from,to,via-)
  set-vars
  @Num n:-zero?
  [
    @Num @From @To @Via
    @Num n:dec @From @Via @To hanoi
    set-vars
    @To @From '\nMove_a_ring_from_%n_to_%n s:with-format puts
    @Num n:dec @Via @To @From hanoi
  ] if ;
~~~

And a test.

~~~
#3 #1 #3 #2 hanoi nl
~~~
