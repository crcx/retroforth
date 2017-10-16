# example|IterativeFibonacci

Declare module constant (prevents reloading when using `import`):

````
:example|IterativeFibonacci ;
````

----

````
:fib (n-m)
  [ #0 #1 ] dip
  [ over + swap ] times drop ;
````
