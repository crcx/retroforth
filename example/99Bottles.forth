# example|99Bottles

Display the text for the *99 Bottles of Beer* song.

Declare module constant (prevents reloading when using `import`):

````
:example|99Bottles ;
````

````
[ dup putn sp 'bottles puts ]
[ '1_bottle puts ]
[ 'no_more_bottles puts  ]
'bottles d:create
  , , ,

:display-bottles
  dup #2 n:min bottles + fetch call ;

:display-beer
  display-bottles '_of_beer puts ;

:display-wall
  display-beer '_on_the_wall puts ;

:display-take
  'Take_one_down,_pass_it_around  puts ;

:display-verse
  display-wall nl display-beer nl
  n:dec display-take nl display-wall nl ;

:?dup
  dup 0; ;

:verses
  [ nl display-verse dup n:-zero? ] while drop ;
 
#99 verses
````
