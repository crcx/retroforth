# RETRO as a Shell: or Shell Scripting in RETRO

It's possible to use RETRO as a Unix shell, though the experience is
quite different than traditional shells. It's not my intent in this
to provide a general purpose shell; rather it is to allow for porting
shell scripts to RETRO so I can use my own tools as much as possible.

I begin by defining some words for dealing with pipes.

~~~
:pipe>  (s-s)  file:R unix:popen [ file:read-line ] [ unix:pclose ] bi ;
:>pipe> (ss-s) swap 'echo_"%s"_|_%s s:format dup s:put nl pipe> ;
:>pipe  (ss-)  >pipe> drop ;
~~~

These make it easier to construct the pipelines. E.g.,

    'date_-u_"+%Y%m%d" pipe> 'sed_s/20// >pipe> s:put nl

I can implement some shell wrapper words (using an `sh:` namespace here
to group them):

~~~
:sh:cp    (source,dest) swap 'cp_%s_%s s:format unix:system ;
:sh:md    (dirname)     'mkdir_%s s:format unix:system ;
:sh:rm    (filename)    'rm_%s s:format unix:system ;
:sh:rm<force,recurse> (filename) 'rm_-rf_%s s:format unix:system ;
:sh:touch (filename)    'touch_%s s:format unix:system ;
:sh:gzip  (filename)    'gzip_%s s:format unix:system ;
:sh:ls                  [ s:put sp ] unix:for-each-file ;
:sh:ls<long>            [ s:put nl ] unix:for-each-file ;
~~~

Wrapping the find tool is a little more involved. I decided to have
this generate a tab separated value string, which I then tokenize into
an array.

This will require that the temporary string length be at least as big
as the returned length.

~~~
:sh:find (path,name-array)
  swap 'find_%s_-name_"%s"_-type_f_-print_|_tr_'\\n'_'\\t' s:format
  pipe> s:chop ASCII:HT s:tokenize ;
~~~

I don't know how to do a direct analog to xargs. This should be
similar in functionality.

~~~
:sh:args (array,command)
  [ s:format pipe> drop ] curry a:for-each ;
~~~
