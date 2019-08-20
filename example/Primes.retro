This is a quick and dirty way to find prime numbers in a set.

~~~
{{
  #2 'NextPrime var<n>
  :extract (s-s)
    [ @NextPrime dup-pair eq?
      [ drop-pair TRUE ]
      [ mod n:-zero? ] choose ] a:filter ;
---reveal---
  :get-primes (s-s)
    #2 !NextPrime
    dup fetch [ extract &NextPrime v:inc ] times ;
}}
~~~

And a test:

~~~
:create-set (-a)
  here #1000 , #2 #1002 [ dup , n:inc ] times drop ;

create-set get-primes [ n:put sp ] a:for-each
~~~
