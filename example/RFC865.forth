This implements a simple socket based server for RFC865, the "Quote of
the Day Protocol".

~~~
'Socket var

{{
  'Sock  var
  'Quote var
  :if-failed  swap [ 'ERROR:_ s:put s:put nl bye ] &drop choose ;
  :bind       n:to-string @Sock socket:bind drop  'binding_to_port  if-failed ;
  :listen     #5 @Sock socket:listen drop         'preparing_socket if-failed ;
  :accept     @Sock socket:accept n:-zero?        'accept_failed    if-failed ;
  :send       '_ [ store ] sip @Socket socket:send drop-pair ;
  :remap      &send &c:put set-hook ;
  :unmap      &c:put unhook ;
  :process    [ remap !Socket @Quote call reset unmap ] sip socket:close ;
---reveal---
  :server:for-each-connection (nq-)
    !Quote socket:create !Sock
    bind listen repeat accept process again ;
}}
~~~

A test case, sending a message and the time to the user.

~~~
#9999 [ here '/etc/motd file:slurp here s:put ] server:for-each-connection
~~~
