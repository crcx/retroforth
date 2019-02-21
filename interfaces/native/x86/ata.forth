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

# Buffer

~~~
'ata:Sector d:create #513 allot
~~~

# Common

~~~
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
:ata:read (n-)
  ata:set-sector
  ata:READ 0x1F7 pio:out-byte
  #10000 [ ] times
  &ata:Sector #256 [ ata:PRIMARY pio:in-word [ 0xFF and over store n:inc ] sip #8 shift over store n:inc ] times drop ;
~~~

# Writing a Sector

~~~
:ata:write (n-)
  ata:set-sector
  ata:WRITE 0x1F7 pio:out-byte
  #10000 [ ] times
  &ata:Sector #256 [ fetch-next [ fetch-next #-8 shift ] dip + ata:PRIMARY pio:out-word ] times drop ;
~~~

# Experiments

~~~
#1024 'ata:cylinders var
 #255 'ata:heads     var
  #63 'ata:sectors   var

{{
  'LBA var
  :HEADS   @ata:heads ;
  :SECTORS @ata:sectors ;
  :H*S   HEADS SECTORS * ;
  :C     @LBA H*S / ;
  :TEMP  @LBA H*S mod ;
  :H     TEMP SECTORS / ; 
  :S     TEMP SECTORS mod n:inc ;
---reveal---
  :lba->chs !LBA C H S ;
}}
~~~

# Todo

- read partition table
- identify a partition for retro
- restrict read/write to this
- support second disk?
- atapi?

