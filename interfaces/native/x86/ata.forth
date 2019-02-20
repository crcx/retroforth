# ATA (Hard Disk) Driver

**This is *very* incomplete. Don't use it yet.**

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

'Sector d:create #513 allot

:ata:read (n-)
  0xE0 0x1F6 pio:out-byte
  0x00 0x1F1 pio:out-byte
  0x01 0x1F2 pio:out-byte
  dup  0x1F3 pio:out-byte
  dup #8 shift 0x1F4 pio:out-byte
     #16 shift 0x1F5 pio:out-byte
  0x20 0x1F7 pio:out-byte
  #10000 [ ] times
  &Sector #256 [ 0x1F0 pio:in-word [ 0xFF and over store n:inc ] sip #8 shift over store n:inc ] times drop ;

:ata:write (n-)
  0xE0 0x1F6 pio:out-byte
  0x00 0x1F1 pio:out-byte
  0x01 0x1F2 pio:out-byte
  dup  0x1F3 pio:out-byte
  dup #8 shift 0x1F4 pio:out-byte
     #16 shift 0x1F5 pio:out-byte
  0x20 0x1F7 pio:out-byte
  #10000 [ ] times
  &Sector #256 [ fetch-next [ fetch-next ] dip #-8 shift + 0x1F0 pio:out-word ] times drop ;

~~~
