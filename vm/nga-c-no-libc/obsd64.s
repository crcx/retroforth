; This is the minimal startup + I/O functionality needed to run
; RETRO on an OpenBSD x86-64 system.
; =============================================================

bits 64

section .note.openbsd.ident
        align   2
        dd      8,4,1
        db      "OpenBSD",0
        dd      0
        align   2

section .text
global putchar
global getchar
global _start

extern main

align 8
_start:
        jmp    main
        jmp     $

align 8
putchar:
        mov     rax, rdi
        mov     [buf], eax
        mov     rax, 4        ; sys_write
        mov     rdi, 1        ; stdout
        mov     rsi, buf      ; address
        mov     rdx, 1        ; 1 byte
        syscall
        ret

align 8
getchar:
        mov     rax, 3        ; sys_read
        mov     rdi, 0        ; stdin
        mov     rsi, buf      ; address
        mov     rdx, 1        ; 1 byte
        syscall
        mov     rax, 0
        mov     eax, [buf]
        ret

section .data
buf:
        dd 0
        dd 0
        dd 0
        dd 0
