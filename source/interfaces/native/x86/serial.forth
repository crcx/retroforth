# Serial Port

This was adapted from the RETRO9 serial driver. To use it,
set the `serial:Port` variable to the port you want to use.
Then do `serial:init` and read/write as necessary.

~~~
#1016 'serial:COM1 const
#760  'serial:COM2 const
#1000 'serial:COM3 const
#744  'serial:COM4 const
serial:COM1 'serial:Port var<n>

:serial:received? @serial:Port #5 + pio:in-byte #1  and n:-zero? ;
:serial:empty?    @serial:Port #5 + pio:in-byte #32 and n:-zero? ;
:serial:read      serial:received? [ @serial:Port pio:in-byte  ] if; serial:read ;
:serial:write     serial:empty?    [ @serial:Port pio:out-byte ] if; serial:write ;
:serial:send      [ serial:write ] s:for-each ;
:serial:init      #0  @serial:Port #1 + pio:out-byte #128 @serial:Port #3 + pio:out-byte
                  #3  @serial:Port      pio:out-byte #0   @serial:Port #1 + pio:out-byte
                  #3  @serial:Port #3 + pio:out-byte #199 @serial:Port #2 + pio:out-byte
                  #11 @serial:Port #4 + pio:out-byte ;
~~~
