# Paste to Sprunge

sprunge.us is a command line pastebin services. It's
normally used like:

    <command> | curl -F 'sprunge=<-' http://sprunge.us

This is a wrapper for sharing snippits from within
RETRO.

## The Code

~~~
{{
  :write  '/tmp/rx.paste file:spew ;
  :curl   'curl_-s_-F_'sprunge=</tmp/rx.paste'_http://sprunge.us ;
  :pipe{  curl file:R unix:popen ;
  :get    here [ #100 [ dup file:read , ] times ] dip ;
  :result s:chop s:temp ;
  :}      swap unix:pclose '/tmp/rx.paste file:delete ;
---reveal---
  :,paste (s-s)
    &Heap [ write pipe{ get result } ] v:preserve ;
}}
~~~

## Test Case

```
{{
  :capture{  &buffer:add &c:put set-hook ;
  :}         &c:put unhook ;
---reveal---
  :capture-output (qa-)
    [ buffer:set capture{ call } ] buffer:preserve ;
}}

'Out d:create #128000 allot

[ d:words ] Out capture-output
Out ,paste s:put nl
```
