# example|RecursiveFibonacci

~~~
:fib (n-m)
  dup
  [ n:zero? ] [ #1 eq? ] bi or
  not 0; drop
  [ n:dec fib ] sip
  [ #2 - fib ] call + ;
~~~
