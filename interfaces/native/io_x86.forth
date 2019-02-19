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

~~~
{{
  'io:PortIO var
  :identify
    @io:PortIO n:zero? [
      #2000 io:scan-for dup n:negative?
      [ drop 'IO_DEVICE_TYPE_2000_NOT_FOUND s:put nl ]
      [ !io:PortIO ] choose ] if ;
  ---reveal---
  :io:portio identify @io:PortIO io:invoke ;
}}
~~~

~~~
:io:inb  (p-n)    #0 io:portio ;
:io:outb (vp-)    #1 io:portio ;
~~~

~~~
#112 'CMOS:ADDRESS const
#113 'CMOS:DATA    const

:rtc:query  CMOS:ADDRESS io:outb CMOS:DATA io:inb ;
:rtc:second #0 rtc:query ;
:rtc:minute #2 rtc:query ;
:rtc:hour   #4 rtc:query ;
:rtc:day    #7 rtc:query ;
:rtc:month  #8 rtc:query ;
:rtc:year   #9 rtc:query ;

:time rtc:hour n:put $: c:put rtc:minute n:put nl ;
~~~

~~~
#1016 'serial:COM1 const
#760  'serial:COM2 const
#1000 'serial:COM3 const
#744  'serial:COM4 const
serial:COM2 'serial:Port var<n>

:serial:port      @serial:Port ;
:serial:received? serial:port #5 + io:inb #1  and n:-zero? ;
:serial:empty?    serial:port #5 + io:inb #32 and n:-zero? ;
:serial:read      serial:received? [ serial:port io:inb  ] if; serial:read ;
:serial:write     serial:empty?    [ serial:port io:outb ] if; serial:write ;
:serial:send      [ serial:write ] s:for-each ;
:serial:init      #0  serial:port #1 + io:outb #128 serial:port #3 + io:outb
                  #3  serial:port      io:outb #0   serial:port #1 + io:outb
                  #3  serial:port #3 + io:outb #199 serial:port #2 + io:outb
                  #11 serial:port #4 + io:outb ;
~~~
