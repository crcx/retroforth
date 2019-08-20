gopher:get

  Data:  asns-n
  Addr:  -
  Float: -

Takes an address, a server, a port, and a selector. Fetch the
resource and store it at address. Return the number of bytes
received.

Class: class:word | Namespace: gopher | Interface Layer: ios

Example #1:

    here 'forthworks.com #70 '/ gopher:get
    here s:put


~~~
  :set      (asns-sns) 'abcd 'bcda reorder buffer:set ;
  :url      (sns-s)    'abc 'cba reorder 'gopher://%s:%n/0%s s:format ;
  :command  (s-s)      'curl_-s_%s s:format ;
  :connect  (s-p)      file:R unix:popen ;
  :read     (p-p)      [ dup file:read dup buffer:add n:zero? ] until ;
  :complete (p-n)      unix:pclose buffer:end buffer:start - ;
---reveal---
  :gopher:get (asns-n)
    [ set url command connect read complete ] buffer:preserve ;
~~~

## Test Case

~~~
'Data d:create #256001 allot
&Data 'forthworks.com #70 '/retro gopher:get
'%n_bytes_read\n s:format s:put
~~~
