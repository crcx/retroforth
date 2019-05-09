#!/usr/bin/env retro

This program generates an HTML index and exports (using the
`export-as-html.forth` example) the samples to HTML. The files
are stored in `/home/crc/public/examples`.

# Configuration

~~~
'/home/crc/public/examples/ 'FILE-PATH s:const
~~~

# Variables

~~~
'FID var
~~~

# Support

This word takes a string and provides a flag of `TRUE` if it
ends in `/`, or `FALSE` otherwise. It leaves the string pointer
on the stack.

~~~
:dir? (s-sf)
  dup s:length over + n:dec fetch $/ eq? ;
~~~

# Words To Create The Index

~~~
:s:put [ @FID file:write ] s:for-each ;
:css   '<style>*{background:#111;color:#fff;font-family:monospace;}</style>
       s:put ;
:dtd   '<!DOCTYPE_html> s:put ;
:title '<title>RETRO_Examples</title> s:put ;
:head  '<head> s:put title css '</head> s:put ;
:link  dup '<a_href="/examples/%s.html">%s</a><br> s:format s:put $. c:put ;
:body  '<body> s:put call '</body> s:put ;
:make  dtd head body ;
~~~

# Generate index.html

~~~
FILE-PATH 'index.html s:append file:W file:open !FID
[ '<h1>Examples</h1><br> s:put
  [ dir? &drop &link choose ] unix:for-each-file nl ] make
@FID file:close
~~~

# Generate HTML Files

~~~
:export FILE-PATH over
        './export-as-html.forth_%s_>%s%s.html s:format unix:system $. c:put ;

[ dir? &drop [ export ] choose ] unix:for-each-file nl
~~~
