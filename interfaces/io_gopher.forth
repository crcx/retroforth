# Gopher

RETRO has Gopher support via `gopher:get`.

Takes:

  destination
  server name
  port
  selector

Returns:

  number of characters read

~~~
#3 'io:Gopher var<n>
:gopher:get #0 @io:Gopher as{ 'ii...... i }as ;
~~~
