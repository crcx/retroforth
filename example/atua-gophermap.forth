#!/usr/bin/env retro

# Atua :: Gophermap Generation

I've been running a Gopher server written in RETRO since
2018. This server, named Atua, has served me quite well.
But it has one limit that sometimes proves annoying: there
is no support for generating a directory listing. Atua
only serves the data in a `gophermap`.

I decided to rectify this in a way. Rather than altering
Atua to add more complexity, I decided to write a tool
which can generate the `gophermap` automatically.

As a practical matter, the list will exclude files named
`gophermap`, `HEADER`, and `FOOTER`. The generated file
will consist of the contents of `HEADER`, the directory
entries, and the contents of `FOOTER`.

Output will be written to standard output. Redirect to the
`gophermap` file, or pipe it to another process for
examination or manipulation.

# The Code

I begin by creating a word to return the number of files in
the current directory. This makes use of a Unix pipe to run
`ls -l | wc -l` and capture the result. I trim off any
whitespace and convert to a number.

~~~
:unix:count-files (-n)
  'ls_-1_|_wc_-l  file:R unix:popen
  [ file:read-line s:trim s:to-number ] [ unix:pclose ] bi ;
~~~

Next, a word to identify the current working directory. This
also uses a pipe to `pwd`.

~~~
:unix:get-cwd (-s)
  'pwd file:R unix:popen
  [ file:read-line s:trim ] [ unix:pclose ] bi '/ s:append ;
~~~

The program accepts a single command line argument: the
physical base path to exclude. In Atua, there is a root
directory, and all selector paths are relative to this.

E.g., if the actual root is `/home/atua/gopherspace/` then
launching this program as:

    atua-gophermap.forth /home/atua/gopherspace

will strip the actual root path off, allowing the selectors
to work as expected.

~~~
#0 sys:argv s:length 'SKIP const
~~~

So with these defined, I define a couple of constants using
them for later use.

~~~
unix:get-cwd SKIP +  'BASE s:const
unix:count-files     'FILES const
~~~

Ok, now for a useful combinator. I want to be able to run
something once for each file or directory in the current
directory. One option would be to read the names and
construct a set, then use `set:for-each`. I decided to take
a different path: I implement a word to open a pipe, read a
single line, then run a quote against it.

With this, something like `ls` can be defined as:

    :ls [ s:put nl ] unix:for-each-file ;

~~~
:unix:for-each-file (q-)
  'ls_-1_-p file:R unix:popen
  unix:count-files-in-cwd
  [ [ file:read-line s:temp over call ] sip ] times
  unix:pclose drop ;
~~~

# Generate The Output

Begin by displaying HEADER (if it exists).

~~~
'HEADER file:exists?
  [ here 'HEADER file:slurp here s:put nl ] if
~~~

Next, list any directories. If a file name ends with a `/`,
I assume it is a directory.

~~~
:dir? (s-sf)
  dup s:length over + n:dec fetch $/ eq? ;
~~~

A directory entry needs the following form:

    0description<tab>selector<newline>

I am using the directory name as the description (with a
trailing slash), and the relative path (without the final
slash) as the selector.

~~~
:selector (filename-selector)
  BASE s:prepend s:chop ;

:dir-entry (filename)
  $1 c:put dup s:put tab selector s:put nl ;

[ dir? &dir-entry &drop choose ] unix:for-each-file
~~~

Next, list files. This is harder because files can have
different types.

I start with a word to decide if the item is a file. This
will ignore directories (ending in a `/`), `HEADER`, `FOOTER`,
and `gophermap` files.

~~~
:file? (s-sf)
  dup 'HEADER    [ FALSE ] s:case
      'FOOTER    [ FALSE ] s:case
      'gophermap [ FALSE ] s:case
  drop dir? not ;
~~~

Then I look to see if it has a file extension.

~~~
:has-extension? (s-sf)
  dup $. s:contains-char? ;
~~~

If there is an extension, it can be mapped to a type code.
I do this with a simple `s:case` construct, defaulting to
a binary (type 9) file if I don't recognize the extension.

~~~
:file-type
  dup $. s:split drop
  '.forth [ $0 ] s:case
  '.md    [ $0 ] s:case
  '.txt   [ $0 ] s:case
  '.htm   [ $h ] s:case
  '.html  [ $h ] s:case
  drop $9 ;
~~~

Finishing up the file listing, the `file-entry` determines
the file type and prints out the appropriate line.

~~~
:selector (filename-selector)
  BASE s:prepend ;

:file-entry (filename)
  has-extension? [ file-type ] [ $9 ] choose
  c:put dup s:put tab selector s:put nl ;

[ file? &file-entry &drop choose ] unix:for-each-file
~~~

End by displaying FOOTER (if it exists).

~~~
'FOOTER file:exists?
  [ here 'FOOTER file:slurp here s:put nl ] if
~~~

# Conclusion

This was a quick little thing that will make using Atua nicer
in the future. The techniques used here can be beneficial in
other filesystem related tasks as well, so I expect to reuse
portions of this code in the future.
