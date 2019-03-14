This is a quick little tool to strip some HTML and leave text behind.

It's not factored, and doesn't filter blocks that don't exactly match
the tags listed in the code. It's still useful for quick looks, and
may serve as a starting point for something more useful later on.

~~~
'Tag d:create #1025 allot
'In-Tag var

#0 sys:argv file:open<for-reading> swap
  [ dup file:read
    $< [ &Tag buffer:set &In-Tag v:on  ] case
    $> [ &Tag 'head   [ here buffer:set ] s:case
              'title  [ here buffer:set ] s:case
              'script [ here buffer:set ] s:case
              'script_type="text/javascript" [ here buffer:set ] s:case
              'style  [ here buffer:set ] s:case
         drop &In-Tag v:off ] case
    @In-Tag [ buffer:add ] [ c:put ] choose ] times
file:close
~~~
