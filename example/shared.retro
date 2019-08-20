#!/usr/bin/env retro

This is a little server for the pastebin. Use it with inetd or similar:

    gopher  stream  tcp     nowait/6/30/2   nope    /home/crc/retro/example/shared.forth
    http    stream  tcp     nowait/6/30/2   nope    /home/crc/retro/example/shared.forth

It understands HTTP and Gopher, so can work for either. Use it with the
`share.forth` tool.

~~~
s:get 'REQUEST s:const

:http?   REQUEST #0 #3 s:substr 'GET s:eq? ;
:extract REQUEST #32 s:tokenize #2 + fetch ;
:ok      'HTTP/1.0_200_OK\n s:format s:put ;
:mime    'Content-type:_text/plain\n\n s:format s:put ;
:file    '/home/crc/public%s s:format ;
:read    here swap file:slurp here ;
:missing drop 'Selector_not_found. ;
:get     dup file:exists? [ read ] if; missing ;
:http    ok mime extract file get s:put ;
:gopher  REQUEST file get s:put ;
:serve   http? [ http ] if; gopher ;

serve
~~~
