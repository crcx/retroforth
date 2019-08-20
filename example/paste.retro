#!/usr/bin/env retro

This implements `s:paste`, a string to private pastebin in
RETRO. It uses a Unix pipe to `md5` to create the paste name,
keeping in line with the `share.forth` example code.

It takes a string and returns a pointer to the URL of the
paste.

~~~
{{
  :pipe{        dup 'md5_-s_"%s" s:format file:R unix:popen ;
  :skip-space   dup file:read drop ;
  :skip-info    [ dup file:read $= eq? ] until skip-space ;
  :get-checksum #127 [ dup file:read , ] here &times dip swap ;
  :}            unix:pclose ;
  :cleanup      s:chop s:temp ;
  :save         [ '/home/crc/public/%s s:format file:spew ] sip ;
  :url          'http://forth.works/%s s:format ;
---reveal---
  :s:paste (s-s)
    &Heap [ pipe{ skip-info get-checksum } cleanup save url ] v:preserve ;
}}
~~~

```
'Hello_#forth s:paste s:put nl
```
