;  ___ ___ _____ ___  ___      __  ___            ___ ___ ___
; | _ \ __|_   _| _ \/ _ \    / / | __| _ ___ ___| _ ) __|   \
; |   / _|  | | |   / (_) |  / /  | _| '_/ -_) -_) _ \__ \ |) |
; |_|_\___| |_| |_|_\\___/  /_/   |_||_| \___\___|___/___/___/
;
; This is the minimal startup + I/O functionality needed to run
; RETRO on a FreeBSD x86-64 system.
; =============================================================

bits 64
section .text
global _putchar
global _getchar
global _start
extern _main
global start

align 8
start:
_start:
        call    _main
        jmp     $

align 8
_putchar:
        mov     rax, rdi
        mov     [rel buf], eax
        mov     rax, 0x2000004; sys_write
        mov     rdi, 1        ; stdout
        mov     rsi, buf      ; address
        mov     rdx, 1        ; 1 byte
        syscall
        ret

align 8
_getchar:
        mov     rax, 0x2000003; sys_read
        mov     rdi, 0        ; stdin
        mov     rsi, buf      ; address
        mov     rdx, 1        ; 1 byte
        syscall
        mov     rax, 0
        mov     eax, [rel buf]
        ret

section .data
buf:
        dd 0
        dd 0
        dd 0
        dd 0
