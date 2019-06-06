~~~
{{
  'io:Sockets var
  :identify
    @io:Sockets n:zero? [
      #7 io:scan-for dup n:negative?
      [ drop 'IO_DEVICE_TYPE_0004_NOT_FOUND s:put nl ]
      [ !io:Sockets ] choose ] if ;
  ---reveal---
  :io:socket-operation identify @io:Sockets io:invoke ;
}}
~~~

~~~
:socket:gethostbyname (as-)  #0 io:socket-operation ;
:socket:create        (-n)   #1 io:socket-operation ;
:socket:bind          (-) #2 io:socket-operation ;
:socket:listen        (-) #3 io:socket-operation ;
:socket:accept        (-) #4 io:socket-operation ;
:socket:connect       (-) #5 io:socket-operation ;
:socket:send          (-) #6 io:socket-operation ;
:socket:sendto        (-) #7 io:socket-operation ;
:socket:recv          (-) #8 io:socket-operation ;
:socket:recvfrom      (-) #9 io:socket-operation ;
:socket:close         (n-) #10 io:socket-operation ;
~~~
