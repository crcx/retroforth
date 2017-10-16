# example|RecursiveFactorial

Declare module constant (prevents reloading when using `import`):

````
:example|RecursiveFactorial ;
````

----

````
:<factorial>
  dup #1 -eq? 0; drop
  dup n:dec <factorial> * ;

:factorial
  dup n:zero?
  [ n:inc ]
  [ <factorial> ] choose ;
````
 
