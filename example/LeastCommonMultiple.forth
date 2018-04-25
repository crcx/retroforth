# example|LeastCommonMultiple

~~~
:gcd (ab-n)
  [ tuck mod dup ] while drop ;

:lcm (ab-n)
  dup-pair gcd [ * ] dip / ;
~~~
