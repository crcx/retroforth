# example|GreatestCommonDivisor

Declare module constant (prevents reloading when using `import`):

````
:example|GreatestCommonDivisor ;
````

````
:gcd (ab-n)
  [ tuck mod dup ] while drop ;
````
