# example|LeastCommonMultiple

Declare module constant (prevents reloading when using `import`):

````
:example|LeastCommonMultiple ;
````

----

````
:gcd (ab-n)
  [ tuck mod dup ] while drop ;

:lcm (ab-n)
  dup-pair gcd [ * ] dip / ;
````
