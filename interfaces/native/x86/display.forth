# VGA Text Display

~~~
0xB8000 'VGA-BASE const
#80     'COLUMNS  const
#25     'ROWS     const

'vga:Display d:create
  COLUMNS ROWS * n:inc allot

'vga:Row    var
'vga:Column var

:vga:position (-a)
  @vga:Row COLUMNS * #2 *
  @vga:Column #2 * +
  VGA-BASE + ;

:vga:update-cursor
  @vga:Row COLUMNS * @vga:Column + dup
  0x0F 0x3D4 pio:out-byte
  0xFF and 0x3D5 pio:out-byte
  0x0E 0x3D4 pio:out-byte
  #8 shift 0xFF and 0x3D5 pio:out-byte ;

:vga:move-cursor (rc-)
  !vga:Column !vga:Row vga:update-cursor ;

:all-but-top
  [ &vga:Display buffer:set
     VGA-BASE COLUMNS #2 * + ROWS n:dec COLUMNS * [ dup ram:fetch-byte buffer:add #2 + ] times drop ] buffer:preserve ;

:move-up
  &VGA-BASE &vga:Display [ over ram:store-byte #2 + ] s:for-each drop ;

:erase-last-line
  &VGA-BASE ROWS n:dec COLUMNS * #2 * +  COLUMNS [ ASCII:SPACE over ram:store-byte #2 + ] times drop ;

:wrap
  @vga:Row ROWS eq? [
    all-but-top move-up erase-last-line
    ROWS n:dec #0 vga:move-cursor
  ] if
  vga:update-cursor ;

:vga:next
  &vga:Column v:inc
  @vga:Column COLUMNS gt? [ &vga:Row v:inc #0 !vga:Column ] if
  wrap ;

:putc (c-)
  #10 [ #0 !vga:Column &vga:Row v:inc wrap ] case
  #13 [ #0 !vga:Column &vga:Row v:inc wrap ] case
  #8  [ &vga:Column v:dec #32 putc &vga:Column v:dec vga:update-cursor ] case
  vga:position ram:store-byte vga:next ;

:clear (-)
  VGA-BASE COLUMNS ROWS * [ ASCII:SPACE over ram:store-byte #2 + ] times drop
  #0 !vga:Row #0 !vga:Column vga:update-cursor ;

:test #1793 &c:put #2 + store &putc &c:put #3 + store ;
~~~
