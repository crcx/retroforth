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
{{
  'io:Gopher var
  :identify
    @io:Gopher n:zero? [
      #5 io:scan-for dup n:negative?
      [ drop 'IO_DEVICE_TYPE_0005_NOT_FOUND s:put nl ]
      [ !io:Gopher ] choose ] if ;
  ---reveal---
  :gopher:get identify #0 @io:Gopher io:invoke ;
}}
~~~
