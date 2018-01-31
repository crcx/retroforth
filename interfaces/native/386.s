;  ___ ___ _____ ___  ___      __  ____ ___   __
; | _ \ __|_   _| _ \/ _ \    / / |__ /( _ ) / /
; |   / _|  | | |   / (_) |  / /   |_ \/ _ \/ _ \
; |_|_\___| |_| |_|_\\___/  /_/   |___/\___/\___/
;
; This is the minimal startup & I/O needed to run a basic RETRO
; instance on x86 hardware.
; =============================================================

MODULEALIGN equ 1<<0
MEMINFO equ 1<<1
FLAGS equ MODULEALIGN | MEMINFO
MAGIC equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .mbheader
align 4
MultiBootHeader:
  dd MAGIC
  dd FLAGS
  dd CHECKSUM

section .data
gdt:
        dw      (.end-.dummy)
        dd      .dummy
        dw      0
.dummy: dw      0,0,0,0
        dw      0xffff,0x0000
        db      0x00,0x9a,0xcf,0x00
        dw      0xffff,0x0000
        db      0x00,0x92,0xcf,0x00
.end:


section .text
extern main
global loader
global _start
global putchar
global getchar

STACKSIZE equ 0x4000

align 4
loader:
_start:
        lgdt    [gdt]
        mov     esp, stack+STACKSIZE
        push    dword 0
        push    dword 0
        call    vClear
        call    vHome
        jmp     main
        jmp     $

align 4
putchar:
        mov     eax, [esp+4]
        cmp     eax, -1
        jz      clearscreen
        call    vEmit
        ret
clearscreen:
        call    vClear
        call    vHome
        ret

align 4
getchar:
        call    key
        push    eax
        call    vEmit
        pop     eax
        ret

section .text

key:
        xor     eax, eax                ;  clear eax
.a:     in      al, 64h                 ;  Is any data waiting?
        test    al, 1                   ;  Is character = ASCII 0?
        jz      .a                      ;  Yes? Try again
        in      al, 60h                 ;  Otherwise, read scancode
        xor     edx, edx                ;  edx: 0=make, 1=break
        test    al, 80h                 ;  Is character = HEX 80h?
        jz      .b                      ;  Skip the next line
        inc     edx                     ;  Update edx
.b:     and     al, 7Fh                 ;  Filters to handle
        cmp     al, 39h                 ;  the ignored keys
        ja      .a                      ;  We just try another key
        mov     ecx, [board]            ;  Load the keymap
        mov     al, [ecx+eax]           ;  Get the key ASCII char
        or      al, al                  ;  Is is = 0?
        js     .shift                   ;  No, use CAPITALS
        jz     .a                       ;  Ignore 0's
        or     dl, dl                   ;  Filter for break code
        jnz    .a                       ;  Ignore break code
        jmp    .done                    ;  Skip to end
.shift: mov    ecx, [edx*4 + .shifts]   ;  Load the CAPITAL keymap
        mov    [board], ecx             ;  Store into BOARD pointer
        jmp    .a                       ;  And try again
.done:  ret

section .data
.shifts dd shift, alpha
board dd alpha
alpha:
  db 0,27,"1234567890-=",8              ;00-0E
  db 9,"qwertyuiop[]",10                ;0F-1C
  db 0,"asdfghjkl;'`"                   ;1D-29
  db -1,"\zxcvbnm,./",-1,"*",0,32,-2    ;2A-3A
shift:
  db 0,27,"!@#$%^&*()_+",8              ;00-0E
  db 9,"QWERTYUIOP{}",10                ;0F-1C
  db 0,'ASDFGHJKL:"~'                   ;1D-29
  db -1,"|ZXCVBNM<>?",-1,"*",0,32,-2    ;2A-3A


section .text

;---------------------------------------------------------------
; This code is based on the video driver for FTS/Forth. Samuel
; Falvo II has kindly granted permission for me to redistribute
; and modify it without restrictions.  Thank you Sam!
;---------------------------------------------------------------
        ; Current VGA text-mode screen location, along with
        ; some descriptive attributes

video_base:     dd      0xB8000
video_width:    dd      80
video_height:   dd      25

        ; Current VGA text-mode color attribute.
        ; NOTE that it's shifted left by 8 bits, ready for
        ; logical ORing with a character code.

video_attributes: dd     0x0700

        ; Current VGA cursor position.  Also dictates where
        ; the next character will be displayed via EMIT or
        ; TYPE.  These probably don't need to be 32-bits wide,
        ; but it's more convenient for asm coding purposes if
        ; they are.

video_cursorX:  dd      0
video_cursorY:  dd      0

        ; I/O ports for the VGA device in question

video_io:       dd      0x3D4



;****** vChangeAttributes *************************************
;
; NAME
;       vChangeAttributes
;
; SYNOPSIS
;       mov     eax,andMask
;       mov     ebx,xorMask
;       call    vChangeAttributes
;       mov     [oldAttributes],eax
;
; FUNCTION
;       This function can be used to clear, set, replace, or toggle
;       all video attributes flags, depending on the settings of
;       EAX and EBX.  The attribute flags currently defined are as
;       follows:
;
;       Bit 0-7: Used to force character code bits to 1.
;       Bit 8-11: Foreground color
;       Bit 12-14: Background color
;       Bit 15: Blink flag
;       Bit 16-31: Unused -- leave clear.
;
;       Note that for normal text output operation, bits 7-0 must be
;       left 0.
;
; INPUTS
;       EAX     This register contains the AND-mask.  This mask is
;               applied first.  Typically used to "zero out" a sub-field
;               to be set via EBX.
;
;       EBX     This register contains the XOR-mask.  This mask is
;               applied second.
;
; RESULT
;       EAX contains the *former* set of attributes.
;
; NOTE
;       To query the current attributes without changing them,
;       clear EAX and EBX, like this:
;
;       xor eax,eax
;       xor ebx,ebx
;       call vChangeAttributes
;       mov [currentAttributes],eax
;
; BUGS
;
; SEE ALSO
;
;**************************************************************
vChangeAttributes:
        xchg    eax,[video_attributes]
        and     [video_attributes],eax
        xor     [video_attributes],ebx
        ret

;****** vRethinkCursor ****************************************
;
; NAME
;       vRethinkCursor
;
; SYNOPSIS
;       call    vRethinkCursor
;
; FUNCTION
;       Updates the video hardware to relocate the displayed
;       text cursor to match expectations of the running environment.
;
; INPUTS
;
; RESULT
;
; BUGS
;       This function is undefined if video_cursorX and/or
;       video_cursorY is outside the normal addressible bounds
;       of the current display.
;
; SEE ALSO
;       vCR, vLF, vMoveTo
;
;**************************************************************

vRethinkCursor:
        push    eax
        push    ebx
        push    edx

        ; Compute offset
        ; ASSUMES: 0 <= video_cursorX < video_width
        ; ASSUMES: 0 <= video_cursorY < video_height
        mov     eax,[video_cursorY]
        imul    eax,[video_width]
        add     eax,[video_cursorX]
        mov     ebx,eax

        ; Write low offset byte
        mov     al, 0x0f
        mov     dx, [video_io]
        out     dx, al
        mov     al, bl          ;AL = offset low
        inc     edx             ;0x3d5
        out     dx, al

        ; Write high offset byte
        mov     al, 0x0e        ;Index reg.: cursor pos (high byte)
        dec     edx             ;0x3d4
        out     dx, al
        mov     al, bh          ;AL = offset high
        inc     edx             ;0x3d5
        out     dx, al

        pop     edx
        pop     ebx
        pop     eax
        ret

;****** vClear ************************************************
;
; NAME
;       vClear
;
; SYNOPSIS
;       call    vClear
;
; FUNCTION
;       Fills the entire screen with space characters, using the
;       currently set attribute.  The cursor position is NOT
;       affected by this call.
;
; INPUTS
;
; RESULT
;
; BUGS
;
; SEE ALSO
;       vHome
;
;**************************************************************

vClear:
        push    eax
        push    ebx
        push    ecx

        mov     eax,[video_base]
        mov     ebx,[video_height]
        imul    ebx,[video_width]
        mov     ecx,[video_attributes]
        or      ecx,0x20

.more:  mov     [eax],cx
        add     eax,2
        dec     ebx
        or      ebx,ebx
        jnz     .more

        pop     ecx
        pop     ebx
        pop     eax
        ret

;****** vHome *************************************************
;
; NAME
;       vHome
;
; SYNOPSIS
;       call    vHome
;
; FUNCTION
;       Locates the cursor in the upper left-hand corner of the
;       display.
;
; INPUTS
;
; RESULT
;
; BUGS
;
; SEE ALSO
;       vCR, vLF, vClear
;
;**************************************************************

vHome:
        mov     [video_cursorY],dword 0
        ; fall-through to vCR intentional


;****** vCR ***************************************************
;
; NAME
;       vCR
;
; SYNOPSIS
;       call vCR
;
; FUNCTION
;       Performs the glass-tty equivalent of a carriage return.
;
; INPUTS
;
; RESULT
;
; SEE ALSO
;       vLF, vHome
;
;**************************************************************

vCR:
        mov     [video_cursorX],dword 0
        jmp     vRethinkCursor

;****** vLF ***************************************************
;
; NAME
;       vLF
;
; SYNOPSIS
;       call    vLF
;
; FUNCTION
;       Performs the glass-tty equivalent of a line feed.  Note
;       that the horizontal cursor position is NOT affected by
;       this function.
;
; INPUTS
;
; RESULT
;
; BUGS
;
; SEE ALSO
;       vHome, vCR
;
;**************************************************************

vLF:
        push    eax

        mov     eax,[video_cursorY]
        inc     eax
        cmp     eax,[video_height]
        jb      .scrolling_unnecessary
        dec     eax
        call    vScrollUp

.scrolling_unnecessary:
        mov     [video_cursorY],eax
        call    vRethinkCursor

        pop     eax
        call    vCR
        ret

;****** vScrollUp *********************************************
;
; NAME
;       vScrollUp
;
; SYNOPSIS
;       call vScrollUp
;
; FUNCTION
;       Scrolls the whole screen up one line, filling in the bottom
;       with spaces in the currently set attribute.  This function
;       does NOT change the current cursor location.
;
; INPUTS
;
; RESULT
;
; BUGS
;
; SEE ALSO
;       vChangeAttributes, vCR, vLF
;
;**************************************************************

vScrollUp:
        push    eax
        push    ebx
        push    ecx

        ; Scroll the display up one line

        xor     eax,eax                 ; Get offset for (0,1)
        lea     ebx,[eax+1]
        call    vOffsetAt               ; EAX := source

        mov     ebx,[video_base]        ; EBX := destination address

        mov     ecx,[video_width]       ; ECX := number of bytes to move
        imul    ecx,[video_height]
        sub     ecx,[video_width]
        shl     ecx,1

        call    uMoveDown

        ; Overwrite the garbage at the bottom.

        xor     eax,eax
        mov     ebx,[video_height]
        dec     ebx
        call    vOffsetAt

        mov     ebx,[video_width]
        mov     ecx,[video_attributes]
        or      cl,0x20

.more:  mov     [eax],cx
        add     eax,2
        dec     ebx
        or      ebx,ebx
        jnz     .more

        pop     ecx
        pop     ebx
        pop     eax
        ret

;****** vOffsetAt *********************************************
;
; NAME
;       vOffsetAt
;
; SYNOPSIS
;       mov     eax,Xcoord
;       mov     ebx,Ycoord
;       call    vOffsetAt
;       mov     [memLocation],eax
;
; FUNCTION
;       Given an X and Y coordinate on the display (with the upper-
;       left-hand corner set to (0,0)), return a memory address
;       with which a character and attribute pair can be stored.
;
; INPUTS
;       EAX     The horizontal coordinate
;       EBX     The vertical coordinate
;
; RESULT
;       Byte address of the corresponding character in the video
;       frame buffer.
;
; BUGS
;       Strange things happen if the X or Y coordinate are out of
;       bounds for the current VGA frame buffer size.
;
; SEE ALSO
;
;**************************************************************

vOffsetAt:
        push    ebx
        imul    ebx,[video_width]
        add     eax,ebx
        shl     eax,1
        add     eax,[video_base]
        pop     ebx
        ret

;****** vEmit *************************************************
;
; NAME
;       vEmit
;
; SYNOPSIS
;       mov     al,charToEmit
;       call    vEmit
;
; FUNCTION
;       Displays the character in al to the screen at the current
;       cursor position.  This function DOES interpret the following
;       control codes:
;
;       $08     Backspace -- moves cursor to the left one position
;       $09     Tab -- moves cursor to next tab stop (every 8 chars)
;       $0A     Line feed -- advances cursor down one line
;       $0C     Form feed -- clears the screen and homes the cursor
;       $0D     Carriage Return -- Restores cursor to column 0
;
; INPUTS
;       AL      Character to emit to the display
;
; RESULT
;
; BUGS
;
; SEE ALSO
;       vRawEmit
;
;**************************************************************


vEmit:
        cmp     al,0x08
        jz      vBS

        cmp     al,0x09
        jz      vTAB

        cmp     al,0x0A
        jz      vLF

        cmp     al,0x0C
        jz      vFF

        cmp     al,0x0D
        jz      vCR

        ; flow-through to vRawEmit is intentional.

;****** vRawEmit **********************************************
;
; NAME
;       vRawEmit
;
; SYNOPSIS
;       mov     al,charToEmit
;       call    vRawEmit
;
; FUNCTION
;       Displays the character in al to the screen at the current
;       cursor position.  This function DOES NOT interpret any
;       control codes.
;
; INPUTS
;       AL      Character to emit to the display
;
; RESULT
;
; BUGS
;
; SEE ALSO
;       vEmit
;
;**************************************************************



vRawEmit:
        push    eax
        push    ebx
        push    ecx

        and     eax,0x000000FF
        or      eax,[video_attributes]
        mov     ecx,eax
        mov     eax,[video_cursorX]
        mov     ebx,[video_cursorY]
        call    vOffsetAt
        mov     [eax],cx

        pop     ecx
        pop     ebx
        pop     eax

        ; fall-through to vBumpCursor is intentional.

;****** vBumpCursor *******************************************
;
; NAME
;       vBumpCursor
;
; SYNOPSIS
;       call    vBumpCursor
;
; FUNCTION
;       Advances the cursor one character to the right.
;       If wrap-around occurs, it will further drop the cursor
;       down one line, if possible.  If not, the screen is
;       scrolled.
;
; INPUTS
;
; RESULT
;
; BUGS
;
; SEE ALSO
;       vEmit, vRawEmit, vLF, vCR
;
;**************************************************************


vBumpCursor:
        push    eax

        mov     eax,[video_cursorX]
        inc     eax
        cmp     eax,[video_width]
        jnb      .wrapAround
        mov     [video_cursorX],eax
        pop     eax
        jmp     vRethinkCursor

.wrapAround:
        pop     eax
        call    vCR
        jmp     vLF

;****** vType *************************************************
;
; NAME
;       vType
;
; SYNOPSIS
;       mov     eax,msgBuffer
;       mov     ebx,msgLength
;       call    vType
;
; FUNCTION
;       Displays the text stored in msgBuffer, consisting of msgLength
;       bytes.  It is safe to type a buffer of zero bytes.
;
; INPUTS
;       EAX     Pointer to the buffer to display
;       EBX     Length of the buffer.  If 0, no text is displayed.
;
; RESULT
;
; BUGS
;
; SEE ALSO
;       vEmit, vRawEmit
;
;**************************************************************


vType:
        push    eax
        push    ebx
        push    ecx

        mov     ecx,eax

.again: or      ebx,ebx
        jz      .done
        mov     al,[ecx]
        inc     ecx
        call    vEmit
        dec     ebx
        jmp     .again

.done:  pop     ecx
        pop     ebx
        pop     eax
        ret

;****** vBS ***************************************************
;
; NAME
;       vBS
;
; SYNOPSIS
;       call vBS
;
; FUNCTION
;       Emulates the glass-tty equivalent of a backspace on a
;       teletype by moving the cursor to the left, if possible.
;
; INPUTS
;
; RESULT
;
; BUGS
;
; SEE ALSO
;       vTAB
;
;**************************************************************

vBS:    push    eax
        call    vRawBS
        mov     al,' '
        call    vEmit
        pop     eax

        ; fall-through to vBumpCursor is intentional.

vRawBS: push    eax
        mov     eax,[video_cursorX]
        or      eax,eax
        jz      .asFarAsItllGo
        dec     eax
        mov     [video_cursorX],eax
        call    vRethinkCursor
.asFarAsItllGo:
        pop     eax
        ret

;****** vTAB **************************************************
;
; NAME
;       vTAB
;
; SYNOPSIS
;       call vTAB
;
; FUNCTION
;       Advances the cursor to the next tabstop, which is currently
;       every 8 characters.
;
; INPUTS
;
; RESULT
;
; BUGS
;
; SEE ALSO
;       vBS, vCR, vLF
;
;**************************************************************

vTAB:   push    eax
        mov     eax,[video_cursorX]
        and     eax,7
        jz      .moveAnother8

.oneMoreSpace:
        mov     al,0x20
        call    vEmit
        mov     eax,[video_cursorX]
        and     eax,7
        jnz     .oneMoreSpace
        pop     eax
        ret

.moveAnother8:
        mov     al,0x20
        call    vEmit
        call    vEmit
        call    vEmit
        call    vEmit
        call    vEmit
        call    vEmit
        call    vEmit
        call    vEmit
        pop     eax
        ret


;****** vPage / vFF *******************************************
;
; NAME
;       vPage
;
; AKA
;       vFF
;
; SYNOPSIS
;       call    vPage   ; or...
;       call    vFF
;
; FUNCTION
;       Clears the screen and homes the cursor, thus emulating
;       a form-feed operation on a teletype (hence the name; it
;       delivers a "new page").
;
; INPUTS
;
; RESULT
;
; BUGS
;
; SEE ALSO
;       vTAB, vCR, vLF, vEmit
;
;**************************************************************

vPage:
vFF:
        call    vClear
        jmp     vHome


;****** uMoveDown *********************************************
;
; NAME
;       uMoveDown
;
; SYNOPSIS
;       mov     eax,srcAddress
;       mov     ebx,dstAddress
;       mov     ecx,nbrOfBytes
;       call    uMoveDown
;
; FUNCTION
;       Moves ECX bytes from EBX to EAX.  This move is assumed
;       correct if and only if EBX <= EAX.  Otherwise, memory
;       is filled with a repeating pattern.
;
; INPUTS
;       EAX     source address
;       EBX     destination address
;       ECX     number of bytes to move
;
; RESULT
;
; BUGS
;
; SEE ALSO
;       uMoveUp, uMove
;
;**************************************************************

uMoveDown:
        push    eax
        push    ebx
        push    ecx
        push    edx

        cmp     ecx,4
        jb      .no32bits

.more32bits:
        mov     edx,[eax]
        add     eax,4
        mov     [ebx],edx
        add     ebx,4
        sub     ecx,4
        cmp     ecx,4
        jae     .more32bits

.no32bits:
        cmp     ecx,2
        jb      .no16bits


.more16bits:
        mov     dx,[eax]
        add     eax,2
        mov     [ebx],dx
        add     ebx,2
        sub     ecx,2
        cmp     ecx,2
        jae     .more16bits

.no16bits:
        or      ecx,ecx
        jz      .done

.morebytes:
        mov     dl,[eax]
        inc     eax
        mov     [ebx],dl
        inc     ebx
        dec     ecx
        or      eax,eax
        jnz     .morebytes


.done:
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        ret




section .bss
align 4
stack:
  resb STACKSIZE
stack_ptr:
