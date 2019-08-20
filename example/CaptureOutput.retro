# Capturing Output

By taking advantage of the hook in `c:put`, it's possible to
write a combinator to capture output to a user specified
buffer.

~~~
{{
  :capture{  &buffer:add &c:put set-hook ;
  :}         &c:put unhook ;
---reveal---
  :capture-output (qa-)
    [ buffer:set capture{ call } ] buffer:preserve ;
}}
~~~

## A Test Case

```
'Output d:create #256 #1024 * n:inc allot
[ d:words ] &Output capture-output
Output s:length n:put nl
```
