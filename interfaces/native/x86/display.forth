# VGA Text Display

This implements a text display driver for a standard VGA display
on x86 hardware. The exposed words will be in a `vga:` prefix.

First, define the memory region and screen dimensions. Adjust
these if your system is non-standard.

~~~
0xB8000 'VGA-BASE const
#80     'COLUMNS  const
#25     'ROWS     const
~~~

~~~
0x3D4  'VGA-CURSOR const
0x3D5  'VGA-DATA   const
~~~

Next, a couple of variables to track the cursor position.

~~~
'vga:Row    var
'vga:Column var
~~~

~~~
:vga:position (-a)
  @vga:Row COLUMNS * #2 * @vga:Column #2 * + VGA-BASE + ;

:vga:update-cursor
  @vga:Row COLUMNS * @vga:Column + dup
  0x0F              VGA-CURSOR pio:out-byte
  0xFF and          VGA-DATA   pio:out-byte
  0x0E              VGA-CURSOR pio:out-byte
  #8 shift 0xFF and VGA-DATA   pio:out-byte ;

:vga:move-cursor (rc-)
  !vga:Column !vga:Row vga:update-cursor ;

{{
  'vga:Display d:create
    COLUMNS ROWS * n:inc allot

  :all-but-top
    [ &vga:Display buffer:set
       VGA-BASE COLUMNS #2 * + ROWS n:dec COLUMNS * [ dup ram:fetch-byte buffer:add #2 + ] times drop ] buffer:preserve ;

  :move-up
    &VGA-BASE &vga:Display [ over ram:store-byte #2 + ] s:for-each drop ;

  :erase-last-line
    &VGA-BASE ROWS n:dec COLUMNS * #2 * +  COLUMNS [ ASCII:SPACE over ram:store-byte #2 + ] times drop ;
---reveal---
  :vga:wrap
    @vga:Row ROWS eq? [
      all-but-top move-up erase-last-line
      ROWS n:dec #0 vga:move-cursor
    ] if
    vga:update-cursor ;
}}

{{
  :vga:next
    &vga:Column v:inc
    @vga:Column COLUMNS gt? [ &vga:Row v:inc #0 !vga:Column ] if
    vga:wrap ;
---reveal---
  :vga:c:put (c-)
    #10 [ #0 !vga:Column &vga:Row v:inc vga:wrap ] case
    #13 [ #0 !vga:Column &vga:Row v:inc vga:wrap ] case
    #8  [ &vga:Column v:dec #32 vga:c:put &vga:Column v:dec vga:update-cursor ] case
    vga:position ram:store-byte vga:next ;
}}

:clear (-)
  #0 #0 vga:move-cursor
  VGA-BASE COLUMNS ROWS * [ ASCII:SPACE over ram:store-byte #2 + ] times drop
  #0 #0 vga:move-cursor ;

:vga:initialize #1793 &c:put #2 + store &vga:c:put &c:put #3 + store ;
~~~

