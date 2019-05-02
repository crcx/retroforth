# ATA (Hard Disk) Driver

The code here works (at least under qemu), but is *very*
dangerous to use. It will allow you to read or write a
sector to/from a dedicated `ata:Sector` buffer. No checks
are made to validate the sector number. Using these (esp.
`ata:write`) is likely to cause data loss.

# Constants

~~~
0x20 'ata:READ const
0x30 'ata:WRITE const

0x1F0 'ata:PRIMARY const
~~~

# Common

~~~
:ata:clear-bsy (-)
  [ 0x1F7 pio:in-byte 0x80 and n:zero? ] until ;

:ata:set-sector (n-)
  0xE0 0x1F6 pio:out-byte
  0x00 0x1F1 pio:out-byte
  0x01 0x1F2 pio:out-byte
  dup  0x1F3 pio:out-byte
  dup #8 shift 0x1F4 pio:out-byte
     #16 shift 0x1F5 pio:out-byte ;
~~~

# Reading a Sector

~~~
:ata:read (an-)
  ata:set-sector
  ata:READ 0x1F7 pio:out-byte
  #10000 [ ] times
  #256 [ ata:PRIMARY pio:in-word [ 0xFF and over store n:inc ] sip #8 shift over store n:inc ] times drop ;
~~~

# Writing a Sector

~~~
:ata:write (an-)
  ata:set-sector
  ata:WRITE 0x1F7 pio:out-byte
  #10000 [ ] times
  #256 [ fetch-next [ fetch-next #-8 shift ] dip + ata:PRIMARY pio:out-word ] times drop
  0xE7 (cache_flush) 0x1F7 pio:out-byte ata:clear-bsy ;
~~~
