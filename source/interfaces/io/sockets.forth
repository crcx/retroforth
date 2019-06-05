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
:socket:gethostbyname (as-)
  #0 io:socket-operation ;
~~~
