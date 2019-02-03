#!/usr/bin/env retro

## Overview

Casket is an HTTP server written in RETRO.

## History

  v0   Barebones, HTTP/1.0 server for a single file
  v1   Added support for multiple HTML files
  v2   Added support for non-HTML files
  v3   HTTP/1.1, support virtual hosts
  v4   Support 404 error code for file not found

## Casket v4

This is a small HTTP/1.1 server.

First, some configuration options. Since this will run under
inetd there's no need to specify the port. But the path to the
files to serve is rather useful, so define it here.

~~~
'/root/web 'WEBROOT s:const
~~~

Next, I need to handle the incoming requests. In v0 these were
just discarded, but here we actually want to store the request

So an incoming request will look like:

    GET / HTTP/1.1
    Host: retroforth.org

With the lines ending in a CR,LF sequence.

I need to allocate space for the data I care about. There are
two items:

- The `Requested` file
- The desired virtual `Host`

~~~
'Requested d:create #1025 allot
'Host      d:create #1025 allot
~~~

The header processor will read each item and store the `Host`
and `Requested` file. Everything else is ignored.

I implement `eot?` to decide if a line (or field) indicator
has been reached. This is used by `s:get` to decide when the
input should stop. `s:get` records the characters read into
the specified buffer. And finally, `read-request` reads the
input.

~~~
'Done var

:eot? (c-f)
  [ ASCII:CR eq? ]
  [ ASCII:LF eq? ]
  [ ASCII:SPACE eq? ] tri or or ;

:s:get (a-)
  buffer:set [ c:get [ buffer:add ] [ eot? ] bi ] until 
  buffer:get drop ;

:read-request (-)
  [ here s:get
    here s:to-upper 'GET   s:eq? [ &Requested s:get &Done v:inc ] if
    here s:to-upper 'HOST: s:eq? [ &Host      s:get &Done v:inc ] if
    @Done #2 eq? ] until ;
~~~

Next is reading in the desired file. An initial request may be
just a **/**. In this case, Casket will replace the `Requested`
filename with **/index.html**. In the odd case that a file is
requested without a leading **/**, I have a word to add this.
And then a word that constructs a filename.

~~~
:map-/-to-index (-)
  &Requested '/ s:eq?
    [ '/index.html &Requested s:copy ] if ;

:ensure-leading-/ (-)
  @Requested $/ -eq?
    [ '/ &Requested s:append s:keep &Requested s:copy ] if ;

:filename (-s)
  map-/-to-index ensure-leading-/
  &Requested &Host WEBROOT '%s/%s%s s:format ;
~~~

Next, I need to determine the file type. I'll do this by taking
a look at the extension, and mapping this to a MIME type.

~~~
:get-mime-type (-s)
  filename [ $. s:index-of ] sip +
  (textual_files)
    '.txt   [ 'text/plain               ] s:case
    '.md    [ 'text/markdown            ] s:case
    '.htm   [ 'text/html                ] s:case
    '.html  [ 'text/html                ] s:case
    '.css   [ 'text/css                 ] s:case
    '.c     [ 'text/plain               ] s:case
    '.h     [ 'text/plain               ] s:case
    '.forth [ 'text/plain               ] s:case
    '.retro [ 'text/plain               ] s:case
  (image_files)
    '.png   [ 'image/png                ] s:case
    '.jpg   [ 'image/jpeg               ] s:case
    '.jpeg  [ 'image/jpeg               ] s:case
    '.gif   [ 'image/gif                ] s:case
    '.bmp   [ 'image/bmp                ] s:case
  (application_files)
    '.pdf   [ 'application/pdf          ] s:case
    '.gz    [ 'application/gzip         ] s:case
    '.zip   [ 'application/zip          ] s:case
    '.json  [ 'application/json         ] s:case
    '.js    [ 'application/x-javascript ] s:case
    '.xml   [ 'application/xml          ] s:case
  drop 'text/plain ;
~~~

Using these, I can construct a word to read in the file and
send it to the client.

Reading files is now a bit more involved, since images and
other formats have binary data.

~~~
'FID var

:read-file (-an)
  here
  filename file:R file:open !FID
  @FID file:size [ [ @FID file:read , ] times ] sip
  @FID file:close ;

:eol  (-)   ASCII:CR c:put ASCII:LF c:put ;

:send-file (-)
  get-mime-type 'Content_type:_%s s:format s:put eol eol
  read-file [ fetch-next c:put ] times drop ;
~~~

In the above, `eol` will send an end of line sequence.

The last support word is a handler for 404 errors. This
will send the 404 status code and a human readable error
message.

~~~
:404 'HTTP/1.1_404_OK s:put eol
     'Content-type:_text/html s:put eol eol
     'ERROR_404:_FILE_NOT_FOUND s:put eol ;
~~~

And now for the top level server.

Receive a request:

~~~
read-request
~~~

See if the file exists:

~~~
filename file:exists?
~~~

Send an "200 OK" response and the file (or a 404 if
the file wasn't found):

~~~
[ 'HTTP/1.1_200_OK s:put eol send-file ]
[ 404 ] choose
~~~

And v4 is done.
