# net:fetch

This is a simple wrapper over `curl` (https://curl.haxx.se). It
uses the `unix:popen` word to initiate a pipe to curl and stores
the returned data into a buffer.

This takes in three values:

- the URL to fetch
- the destination buffer
- a maximum number of bytes to read

The buffer should be at least one cell longer than the max to
allow for the NULL termination.

## The Code

~~~
:net:fetch (san-n)
  [ [ buffer:set
      'curl_-s_%s s:format file:R unix:popen ] dip
    [ dup file:read 0; buffer:add ] times unix:pclose
    buffer:start s:length ] buffer:preserve ;
~~~

## Test Case

```
'Data d:create #256001 allot
'gopher://forthworks.com &Data #300009 net:fetch
'%n_bytes_read\n s:format s:put
```
