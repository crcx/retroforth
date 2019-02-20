# VGA Text Display

~~~
0xB8000 'VGA-BASE const
#80     'COLUMNS  const
#25     'ROWS     const

'vga:Row    var
'vga:Column var

:vga:position (-a)
  @vga:Row COLUMNS * #2 *
  @vga:Column #2 * +
  VGA-BASE + ;

:cursor
  @vga:Row COLUMNS * @vga:Column + dup
  0x0F 0x3D4 pio:out-byte
  0xFF and 0x3D5 pio:out-byte
  0x0E 0x3D4 pio:out-byte
  #8 shift 0xFF and 0x3D5 pio:out-byte ;

:scroll
  @vga:Row ROWS gt? [ #0 !vga:Row ] if cursor ;

:vga:next
  &vga:Column v:inc
  @vga:Column COLUMNS gt? [ &vga:Row v:inc #0 !vga:Column ] if
  @vga:Row ROWS gt? [ #0 !vga:Row ] if cursor ;

:putc (c-)
  #10 [ #0 !vga:Column &vga:Row v:inc scroll ] case
  #13 [ #0 !vga:Column &vga:Row v:inc scroll ] case
  vga:position ram:store-byte vga:next ;

:clear (-)
  VGA-BASE COLUMNS ROWS * [ ASCII:SPACE over ram:store-byte #2 + ] times drop
  #0 !vga:Row #0 !vga:Column cursor ;

:test      &putc &c:put set-hook ;
~~~
