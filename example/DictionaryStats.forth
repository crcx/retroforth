It's sometimes interesting to take some measurements of dictionary names.

Determine the number of words in the dictionary.

~~~
#0 [ drop n:inc ] d:for-each
'%n_names_defined\n s:with-format puts
~~~

Determine the average length of a word name.

~~~
#0 [ d:name s:length + ] d:for-each
#0 [ drop n:inc ] d:for-each
/ 'Average_name_length:_%n\n s:with-format puts
~~~

Longest name are...

~~~
#0 [ d:name s:length n:max ] d:for-each
'Longest_names_are_%n_characters\n s:with-format puts
~~~
