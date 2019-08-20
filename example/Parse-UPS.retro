This code parses a UPS Type 1Z tracking number and returns the
subsections in a readable format.

See http://osiris.978.org/~alex/ups.html for a list of service codes.

~~~
:valid?  [ #0 #2 s:substr '1Z s:eq? ] sip swap ;
:account [ #2 #6 s:substr 'Account:_ swap s:append ] sip ;
:service-code [ #8 #2 s:substr 'Service_code:_ swap s:append ] sip ;
:package-id [ #10 #7 s:substr 'Package_ID:_ swap s:append ] sip ;
:checksum #17 + 'Checksum:_ swap s:append ;
:parse-ups
  valid? [ account service-code package-id checksum
           #4 [ s:put nl ] times ]
         [ drop 'invalid_tracking_number s:put nl ] choose ;
~~~

```
'1Z6x36270342495730 parse-ups
```
