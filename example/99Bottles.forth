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

:.bottles
  dup #2 n:min bottles + fetch call ;

:.beer
  .bottles '_of_beer putsf ;

:.wall
  .beer '_on_the_wall putsf ;

:.take
  'Take_one_down,_pass_it_around  putsf ;

:.verse
  .wall nl .beer nl
  n:dec .take nl .wall nl ;

:?dup
  dup 0; ;

:verses
  [ nl .verse dup n:-zero? ] while drop ;
 
#99 verses
````
