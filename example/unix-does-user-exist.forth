This implements a word to determine if a user exists. This is done by
parsing the results of `finger`.

~~~
:pipe>  (s-s)  file:R unix:popen [ file:read-line ] [ unix:pclose ] bi ;

{{
  :command  'finger_%s_2>&1 s:format ;
  :parse    ASCII:SPACE s:tokenize ;
  :login?   #0 a:th fetch 'Login: s:eq? ;
---reveal---
  :user:exists? (s-f)
    &Heap [ command pipe> parse login? ] v:preserve ;
}}

'crc user:exists?
'fakeuser user:exists?
'root user:exists?
~~~

A second way is to parse the `/etc/passwd` file.

~~~
{{
  :setup  (s-fs)    FALSE swap ': s:append s:keep ;
  :match? (fss-fsf) over s:begins-with? ;  
  :found  (fs-fs)   drop-pair TRUE s:empty s:keep ;
  :check  (fss-fs)  match? [ found ] if ;
  :seek   (fs-fs)   '/etc/passwd [ check ] file:for-each-line ;
---reveal---
  :user:exists? (s-f)
    &Heap [ setup seek drop ] v:preserve ;
}}

'crc user:exists?
'fakeuser user:exists?
'root user:exists?
~~~
