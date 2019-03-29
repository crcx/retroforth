#!/usr/bin/env retro

This program generates an HTML index and exports (using the
`export-as-html.forth` example) the samples to HTML. The files
are stored in `/home/crc/public/examples`.

~~~
:unix:count-files (-n)
  'ls_-1_|_wc_-l  file:R unix:popen
  [ file:read-line s:trim s:to-number ] [ unix:pclose ] bi ;

:unix:for-each-file (q-)
  'ls_-1_-p file:R unix:popen
  unix:count-files-in-cwd
  [ [ file:read-line s:temp over call ] sip ] times
  unix:pclose drop ;

:dir? (s-sf)
  dup s:length over + n:dec fetch $/ eq? ;
~~~

# Generate index.html

~~~
'/home/crc/public/examples/index.html file:W file:open 'FID const

:index:put [ FID file:write ] s:for-each ;

:css
  '<style>*{background:#111;color:#fff;font-family:monospace;}</style>
  index:put ;

:header '<!DOCTYPE_html> index:put css ;

:link dup '<a_href="/examples/%s.html">%s</a><br> s:format index:put $. c:put ;

header
[ dir? &drop &link choose ] unix:for-each-file nl
FID file:close
~~~

# Generate HTML Files

~~~
:export dup './export-as-html.forth_%s_>/home/crc/public/examples/%s.html
        s:format unix:system $. c:put ;

[ dir? &drop [ export ] choose ] unix:for-each-file nl
~~~
