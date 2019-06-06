This uses the new `socket:` words to download a file via Gopher.

Open a file to store the data and a socket.

~~~
'Output.txt file:open<for-writing> 'File var<n>
socket:create dup n:put nl 'Sock var<n>
~~~

Connect to the server.

~~~
'forthworks.com '70 socket:configure
@Sock socket:connect drop
~~~

Next, send the request.

~~~
'/\n\n s:format @Sock socket:send drop-pair
~~~

After this, I can just read in the data, writing it to
the file.

~~~
[ here #1024 @Sock socket:recv  (discard_errno: drop
  here [ @File file:write ] s:for-each
  dup '%n_bytes_received\n s:format s:put
  (check_for_disconnect: n:zero? ] until
~~~

And finally, clean up by closing the socket and file.

~~~
@Sock socket:close
@File file:close
~~~
