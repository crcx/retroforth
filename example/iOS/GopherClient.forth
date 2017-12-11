This is a tiny Gopher client written in RETRO. It's only for the iOS implementation, but could be adapted for others with a little reworking of the file I/O code.

It's intentionally kept minimal, handling only text files and directories. The interface is line oriented.

# General Configuration

`GOPHER-DATA` is the name of the file to store the last downloaded content into.

`WrapPoint` holds the number of characters to display before wrapping output.

There is also a `Results` variable which will point to the data set being displayed.

~~~
'GopherData.txt 'GOPHER-DATA s:const
#34 'WrapPoint var<n>
'Results var
~~~

# Download Data

Specify a server, a port number, and the initial seletor to fetch.

~~~
:grab (sns-)
  s:empty 'abcd 'ddabc reorder gopher:get
  drop GOPHER-DATA file:spew ;
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
    [ #0 !Displayed nl '__________ puts ]
    [ &Displayed v:inc ] choose ;
~~~

Next is a word to display the directory type. This uses a simple case structure to handle things.

~~~
  :type (c-)
    $0 [ 'TXT_ puts ] case
    $1 [ 'DIR_ puts ] case
    drop '____ puts ;
~~~

I define a `line` word to display the line number. This lets me pad the number with a leading number of spaces and have a colon and space following it.

~~~
  :line (n-)
    n:to-string dup
    s:length #4 swap - [ sp ] times
    puts ':_ puts ;
~~~

I now define a custom version of `puts` that will call the `wrap` word defined earlier. I've not used `s:for-each` here as this will be called frequently, so it's a little faster to have a manually constructed loop here.

~~~
  :puts<w/wrap> (s-)
    [ repeat fetch-next 0; putc wrap again ]
    call drop ;
~~~

The next word is `display`. This will tokenize a line and display the `type` and then the rest of the description using `puts<w/wrap>`.

~~~
  :display (s-)
    #0 !Displayed
    ASCII:HT s:tokenize #0 set:nth fetch
    fetch-next type puts<w/wrap> ;
~~~

~~~
---reveal---
~~~

And finally, tie everything together. This will display an index.

~~~
  :display:index (-)
    #0 here GOPHER-DATA file:slurp drop
    ASCII:LF s:tokenize !Results

    @Results fetch #2 - @Results store

    @Results
    [ over line display nl n:inc ]
    set:for-each drop ;
}}
~~~

# Text Viewer

Text files are really easy. We just read the downloaded file into a buffer and display it.

~~~
:display:text (-)
  GOPHER-DATA file:slurp puts nl ;
~~~

# Final Bits

`display-by-type` takes a type identifer and then calls the proper viewer.

~~~
:display-by-type (c-)
  $0 [ display:text  ] case
  $1 [ display:index ] case
  dup putc nl drop ;
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
  @Results swap set:nth fetch
  ASCII:HT s:tokenize
  dup #0 set:nth fetch fetch [
    [ #2 set:nth fetch ]
    [ #3 set:nth fetch s:chop s:to-number ]
    [ #1 set:nth fetch ] tri grab
  ] dip display-by-type ;
~~~

The final thing to do is just call `home` to get started.

~~~
home
~~~

