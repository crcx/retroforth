# CMOS RTC

~~~
#112 'CMOS:ADDRESS const
#113 'CMOS:DATA    const

:rtc:query  CMOS:ADDRESS pio:out-byte CMOS:DATA pio:in-byte ;
:rtc:second #0 rtc:query ;
:rtc:minute #2 rtc:query ;
:rtc:hour   #4 rtc:query ;
:rtc:day    #7 rtc:query ;
:rtc:month  #8 rtc:query ;
:rtc:year   #9 rtc:query ;

:time rtc:hour n:put $: c:put rtc:minute n:put nl ;
~~~
