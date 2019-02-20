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

:scroll
  @vga:Row ROWS gt? [ #0 !vga:Row ] if ;

:vga:next
  &vga:Column v:inc
  @vga:Column COLUMNS gt? [ &vga:Row v:inc #0 !vga:Column ] if
  @vga:Row ROWS gt? [ #0 !vga:Row ] if ;

:putc (c-)
  #10 [ #0 !vga:Column &vga:Row v:inc scroll ] case
  #13 [ #0 !vga:Column &vga:Row v:inc scroll ] case
  vga:position ram:store-byte vga:next ;
:puts (s-) [ putc ] s:for-each ;
:clear (-) COLUMNS ROWS * #2 * [ ASCII:SPACE putc ] times #0 !vga:Row #0 !vga:Column ;

:test      &putc &c:put set-hook ;
:-test     &c:put unhook ;
~~~
