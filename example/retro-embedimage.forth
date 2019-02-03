#!/usr/bin/env retro

Many of the RETRO interface layers embed a copy of the image
into the binary. This is done by converting the image into a
C file that can be included or compiled and linked. The source
includes a tool, `retro-embedimage`, for this. This implements
the tool in RETRO.

First up, I need to load the image to a buffer, so I allocate
space for this now.

~~~
#65535 #4 * 'IMAGE-SIZE const
'Image d:create
  IMAGE-SIZE allot
~~~

Next is reading in the file. This is slightly complicated by
the need to pack the individual bytes into the memory cells.

So, a variable to track the open file ID.

~~~
'FID var
~~~

Then read in a byte.

~~~
:read-byte (n-)  @FID file:read #255 and ;
~~~

Reading in four bytes, I can shift and merge them back into
a single cell.

~~~
:read-cell (-n)
  read-byte    read-byte    read-byte  read-byte
  #-8 shift +  #-8 shift +  #-8 shift + ;
~~~

The next step is a word to return the size of the file in
cells.

~~~
:size (-n) @FID file:size #4 / ;
~~~

And then, using the above, read in the data from the file
to the image buffer.

~~~
:load-image (s-)
  file:R file:open !FID
  &Image size [ read-cell over store n:inc ] times drop
  @FID file:close ;
~~~

Read in the file.

~~~
'ngaImage load-image
~~~

The final part is to export the image as a C array. To keep
line length to a reasonible length, I have a counter and add
a newline after 18 values.

~~~
'Count var
:EOL? &Count v:inc @Count #18 eq? [ nl #0 !Count ] if ;
~~~

The rest is easy. Display the relevant header bits, then
the cells, then the needed footer.

~~~
'#include_<stdint.h>                     s:put nl
'int32\_t_ngaImageCells_=_9139; s:format s:put nl
'int32\_t_ngaImage[]_=_{        s:format s:put nl

&Image #3 + fetch  [ fetch-next n:put $, c:put EOL? ] times

'}; s:put nl
~~~
