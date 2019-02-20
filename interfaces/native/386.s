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
global getchar

STACKSIZE equ 0x4000

align 4
loader:
_start:
        lgdt    [gdt]
        mov     esp, stack+STACKSIZE
        push    dword 0
        push    dword 0
        jmp     main
        jmp     $

align 4
getchar:
        call    key
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


section .bss
align 4
stack:
  resb STACKSIZE
stack_ptr:
