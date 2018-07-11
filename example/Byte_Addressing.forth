# Byte_Addressing

## Authors

2010, Marc Simpson
2011, 2018, Charles Childers

## Description

This adds words allowing access to memory at a byte level.

Nga is cell addressed. There's no direct access to memory at a byte level. To access a specific byte one must fetch a cell, then extract the individual bytes by masking and shifting. Updating bytes is similarly annoying. The words here take care of these details.

## Functions

+----------+--------+----------------------------+
| Function | Stack  | Used For                   |
+==========+========+============================+
| b:unpack | c-bbbb | Given a byte-packed cell   |
|          |        | on the stack, return the   |
|          |        | bytes it contains          |
+----------+--------+----------------------------+
| b:pack   | bbbb-c | Pack four byes into a cell,|
|          |        | return the cell on the     |
|          |        | stack                      |
+----------+--------+----------------------------+
| b:fetch  | a-b    | Fetch a byte               |
+----------+--------+----------------------------+
| b:store  | ba-    | Store a byte into memory   |
+----------+--------+----------------------------+


## Code & Commentary

~~~
{{
  'Byte var

  :byte-mask  (xn-b)
    #255 swap #8 * dup
    [ n:negate shift and ] dip shift ;

  :replace
    #0 [ [ [ [ drop @Byte ] dip ] dip ] dip ] case
    #1 [ [ [ drop @Byte ] dip ] dip ] case
    #2 [ [ drop @Byte ] dip ] case
    #3 [ drop @Byte ] case
    drop ;

---reveal---

  :b:unpack (c-bbbb)
    dup           #255 and swap
    dup  #8 shift #255 and swap
    dup #16 shift #255 and swap
        #24 shift #255 and ;

  :b:pack (bbbb-c)
    #-24 shift   swap
    #-16 shift + swap
    #-8  shift + swap + ;

  :b:fetch (a-b)
    #4 /mod swap
    #4 /mod
    rot + fetch swap byte-mask ;

  :b:store (ba-)
    swap !Byte
    #4 /mod swap [ dup fetch b:unpack ] dip
    replace b:pack swap store ;
}}
~~~

```
&s:length #4 * #0 + b:fetch
&s:length #4 * #1 + b:fetch
&s:length #4 * #2 + b:fetch
&s:length #4 * #3 + b:fetch
```

