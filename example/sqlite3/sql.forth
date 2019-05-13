This is an experimental set of words allowing use of sqlite3
within RETRO.

Do not use this in anything public facing. It doesn't do any
error checking, input validation, or make any attempt to be
remotely secure. It's also slow due to its design.

A lot of work is needed for this to actually be useful outside
of my tests.

This works by opening pipes to sqlite3, capturing the output,
and then processing it from within RETRO. This is slow, but
allows me to not need to deal with adding an FFI, and is
adaptable to similar external tools.

~~~
{{
  'Database var
  :setup  here buffer:set ;
  :pipe   @Database 'sqlite3_%s_'%s' s:format file:R unix:popen ;
---reveal---
  :sql:set-database s:keep !Database ;
  :sql:query
    [ setup pipe [ dup file:read dup buffer:add n:-zero? ] while unix:pclose ] buffer:preserve here ASCII:LF s:tokenize dup v:dec ;
  :sql:split-line (s-a) $| s:tokenize ;
}}
~~~

# Construct an SQL Statement

~~~
{{
  { 'Action 'Choice 'Table 'Clause } [ var ] a:for-each

  :clear  { &Action &Choice &Table &Clause } [ v:off ] a:for-each ;
---reveal---
  :SELECT 'SELECT !Action !Choice ;
  :DROP   'DROP !Action ;
  :FROM   !Table ;
  :WHERE  !Clause ;
  :sql:statement (q-s)
    clear call @Clause n:-zero?
    [ @Clause @Table @Choice @Action '%s_%s_FROM_%s_WHERE_%s ]
    [ @Table @Choice @Action         '%s_%s_FROM_%s          ] choose
    s:format ;
}}
~~~

# Generate Accessor Words

~~~
{{
  'Table var
  :set-table    (s-s) dup s:keep !Table ;
  :get-columns  (s-a) 'PRAGMA_table\_info("%s") s:format sql:query ;
  :id           (a-n) #0 a:fetch s:to-number ;
  :column       (a-s) #1 a:fetch @Table '%s:%s s:format ;
  :generate     (ns-) d:create , [ fetch a:fetch ] does ;
---reveal---
  :sql:accessors-for (s-)
    set-table get-columns
    [ sql:split-line [ id ] [ column ] bi generate ] a:for-each ;
}}
~~~
