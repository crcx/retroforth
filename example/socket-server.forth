Get a socket.

~~~
socket:create 'Sock var<n>
~~~

Bind to port 9998.

~~~
'9998 @Sock socket:bind drop
~~~

Prepare to listen for connections.

~~~
#3 @Sock socket:listen drop-pair
~~~

Serve the user some data. This will repeat 5 times, then end.

~~~
#5 [ @Sock socket:accept (discard_error_code: drop )
     'Hello_from_RETRO\n s:format swap [ socket:send drop-pair ] sip socket:close
   ] times
~~~

Clean up.

~~~
@Sock socket:close
~~~
