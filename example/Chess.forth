# example:Chess

This is the start of a chess game for RETRO.

It's based heavily on an earlier, two player system for Retro 11.

Begin a private namespace.

````
{{ 
````

Setup the default board.

We use an ASCII character for each piece.

| Char | Represents |
| ---- | ---------- |
| r    | Rook       |
| n    | Knight     |
| b    | Bishop     |
| q    | Queen      |
| k    | King       |
| p    | Pawn       |

Lowercase is used for black pieces, UPPERCASE for white.

````
  :str:, (s-)
   [ , ] s:for-each ; 
 
  'Blank d:create
  'rnbqkbnrpppppppp str:,
  '................ str:,
  '................ str:,
  'PPPPPPPPRNBQKBNR str:, #0 ,
````

The default board (named Blank) won't be direcly used. I create a second board for the actual gameplay.

````
  'Board d:create
  #64 allot 
````

Next up, the fundamentals of the display.

It'll display like this:

        0_1_2_3_4_5_6_7
      +-----------------+
    0 | r n b q k b n r |
    1 | p p p p p p p p |
    2 | . . . . . . . . |
    3 | . . . . . . . . |
    4 | . . . . . . . . |
    5 | . . . . . . . . |
    6 | P P P P P P P P |
    7 | R N B Q K B N R |
      +-----------------+
        0_1_2_3_4_5_6_7


So first up, the columns.

````
  :cols (-)
    '____0_1_2_3_4_5_6_7 puts nl ;
````

Then a horizontal separator.

````
  :--- (-) 
    '__+-----------------+ puts nl ;
````

And then a row.

Possible Future:


````
  :row (a-a)
    '%n_|_ s:with-format puts
    #8 [ fetch-next putc sp ] times
    $| putc nl ;
````

These will be tied together a little later into the top level display word.

````
  :get (rc-a) swap #8 * &Board + + ;
---reveal---
  :chess:display (-)
    nl cols ---
    &Board #0 row
           #1 row
           #2 row
           #3 row
           #4 row
           #5 row
           #6 row
           #7 row drop
    --- cols nl ;
  :chess:new (-)
    &Blank &Board #64 copy ;
  :chess:move (rcrc-)
    get [ get ] dip swap
    dup fetch swap $. swap store
    swap store ;
````

Close the private namespace.

````
}}
````

````
  chess:new
  #0 #0 #2 #1 chess:move chess:display
````
