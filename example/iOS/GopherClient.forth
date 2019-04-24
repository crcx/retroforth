This is a tiny Gopher client.

It's intentionally kept minimal, handling only text files and directories. The interface is line oriented.

# General Configuration

`TARGET` is the name of the buffer to store the last downloaded content into.

`WrapPoint` holds the number of characters to display before wrapping output.

There is also a `Results` variable which will point to the data set being displayed.

~~~
#34 'WrapPoint var<n>
'Results var
'TARGET d:create #1024 #64 * allot
~~~

# Download Data

Specify a server, a port number, and the initial seletor to fetch.

~~~
:grab (sns-)
  &TARGET 'abcd 'dabc reorder gopher:get
  drop ;
~~~

# Indexes

An output line will have a format like this:

    1234: TXT About this server
    1235: DIR Archive
    1236:     This is an information line or
              unknown type

I define a variable `Displayed` to track how
many characters have been displayed so far.

~~~
{{
  'Displayed var
~~~

A `wrap` word handles wrapping and indenting lines based on the `Displayed` characters and the global `WrapPoint`.

~~~
  :wrap
    @Displayed @WrapPoint gt?
    [ #0 !Displayed nl '__________ s:put ]
    [ &Displayed v:inc ] choose ;
~~~

Next is a word to display the directory type. This uses a simple case structure to handle things.

~~~
  :type (c-)
    $0 [ 'TXT_ s:put ] case
    $1 [ 'DIR_ s:put ] case
    drop '____ s:put ;
~~~

I define a `line` word to display the line number. This lets me pad the number with a leading number of spaces and have a colon and space following it.

~~~
  :line (n-)
    n:to-string dup
    s:length #4 swap - [ sp ] times
    s:put ':_ s:put ;
~~~

I now define a custom version of `s:put` that will call the `wrap` word defined earlier. I've not used `s:for-each` here as this will be called frequently, so it's a little faster to have a manually constructed loop here.

~~~
  :s:put<w/wrap> (s-)
    [ repeat fetch-next 0; c:put wrap again ]
    call drop ;
~~~

The next word is `display`. This will tokenize a line and display the `type` and then the rest of the description using `s:put<w/wrap>`.

~~~
  :display (s-)
    &Heap [ #0 !Displayed
            ASCII:HT s:tokenize #0 a:nth fetch
            fetch-next type s:put<w/wrap> ] v:preserve ;
~~~

~~~
---reveal---
~~~

And finally, tie everything together. This will display an index.

~~~
  :display:index (-)
    &Heap [
      &TARGET ASCII:LF s:tokenize !Results

      @Results fetch #2 - @Results store

      #0 @Results
      [ over line display nl n:inc ]
      a:for-each drop
    ] v:preserve ;
}}
~~~

# Text Viewer

Text files are really easy. We just display the contents of the download buffer.

~~~
:display:text (-)
  &TARGET s:put nl ;
~~~

# Final Bits

`display-by-type` takes a type identifer and then calls the proper viewer.

~~~
:display-by-type (c-)
  $0 [ display:text  ] case
  $1 [ display:index ] case
  dup c:put nl drop ;
~~~

`home` just fetches a starting index and displays it. I have this hardcoded to my personal Gopher server.

~~~
:home (-)
  'forthworks.com #70 '/ grab
  $1 display-by-type ;
~~~

The last bit is `g`, which is used to navigate a directory. Pass it the line number to load.

~~~
:g (n-)
  &Heeap [ @Results swap a:nth fetch
           ASCII:HT s:tokenize
           dup #0 a:nth fetch fetch [
             [ #2 a:nth fetch ]
             [ #3 a:nth fetch s:chop s:to-number ]
             [ #1 a:nth fetch ] tri grab
           ] dip display-by-type
  ] v:preserve ;
~~~

The final thing to do is just call `home` to get started.

~~~
home
~~~
