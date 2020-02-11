## Fix Annoyances

Underscores in names (especially variables and constants) are a
problem as the string processor (`prefix:'`) replaces them with
a space. The other prefixes do not do this, which leads to bugs.

Consider:

   'test_data var
   #10 !test_data

`test_data` is not found as the real name is `test data`, so it
silently maps the address to 0.

As a solution, this replaces `d:add-header` with an new version
that implementation that remaps any spaces back to underscores
prior to creating the header.

~~~
{{
  :fields        @Dictionary , (link) , (xt) , (class) ;
  :invalid-name? dup ASCII:SPACE s:contains-char? ;
  :rewrite       [ ASCII:SPACE [ $_ ] case ] s:map ;
  :entry         here &call dip !Dictionary ;
  [ [ fields invalid-name? &rewrite if s, (name) ] entry ]
}}

#1793 &d:add-header store
&d:add-header n:inc store
~~~
