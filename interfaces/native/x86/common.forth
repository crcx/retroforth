# x86 Device Drivers

This exposes the low level port I/O device to RETRO. These
will be used in the implementation of the device drivers.

~~~
{{
  'io:X86 var
  :identify
    @io:X86 n:zero? [
      #2000 io:scan-for dup n:negative?
      [ drop 'IO_DEVICE_TYPE_2000_NOT_FOUND s:put nl ]
      [ !io:X86 ] choose ] if ;
  ---reveal---
  :io:x86 identify @io:X86 io:invoke ;
}}
~~~

# I/O Ports

~~~
:pio:in-byte    (p-n)  #0 io:x86 ;
:pio:out-byte   (vp-)  #1 io:x86 ;
:pio:in-word    (p-n)  #6 io:x86 ;
:pio:out-word   (vp-)  #7 io:x86 ;
~~~

# Access to Physical RAM

~~~
:ram:store      (va-)  #2 io:x86 ;
:ram:fetch      (a-v)  #3 io:x86 ;
:ram:store-byte (va-)  #4 io:x86 ;
:ram:fetch-byte (a-v)  #5 io:x86 ;
~~~

# Hexadecimal

Since most resources list the values and ports in hex, I
am defining a prefix to be used. This will allow for hex
values to be specified like `0xC3`. They must be caps, and
negative values are not supported.

~~~
{{
  'Number var
  '0123456789ABCDEF 'DIGITS s:const
  :convert    (c-)  &DIGITS swap s:index-of @Number #16 * + !Number ;
---reveal---
  :prefix:0 (s-n)
    n:inc #0 !Number [ convert ] s:for-each @Number class:data ; immediate
}}
~~~
