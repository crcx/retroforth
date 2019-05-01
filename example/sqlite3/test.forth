This is a test case, using an sqlite3 database to track time off
requests at my employer.

The database schema is:

    CREATE TABLE pto(id integer primary key,
                     month blob, start blob, end blob, year blob, who blob,
                     reason blob, status blob);


Requests are a range, with a start and ending day. Requests for a single
day off have identical start and end days.

~~~
'sql.forth include         (include_the_sqlite3_wrapper
'test.db sql:set-database  (set_the_database_file
'pto sql:accessors-for     (generate_accessors_for_each_column_in_"pto"
~~~

I can query for approved requests (status=y) in the current month.

~~~
[ 'pto FROM '* SELECT 'status="y"_AND_month=5_ORDER_BY_start WHERE ] sql:statement sql:query
~~~

And then iterate over the results, generating some readable output like:

    Charles Childers
       From: 2019-4-18 through 2019-4-22

(This part should be refactored to aid in readability, but this works
ok for a quick test.)

~~~
[ sql:split-line
  dup pto:who s:put nl
  [ &pto:start &pto:month &pto:year tri
    '\tFrom:_%s-%s-%s_through_ s:format s:put ] sip
  [ &pto:end &pto:month &pto:year tri
    '%s-%s-%s\n s:format s:put ] sip
  drop ] a:for-each
~~~
