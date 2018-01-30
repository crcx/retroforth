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
global putchar
global getchar
global _start
extern main

align 8
_start:
	call main
	jmp $

align 8
putchar:
	mov rax, rdi
	mov [buf], eax
	mov rax, 4	; sys_write
	mov rdi, 1	; stdout
	mov rsi, buf	; address
	mov rdx, 1	; 1 byte
	syscall
	ret

align 8
getchar:
	mov rax, 3
	mov rdi, 0
	mov rsi, buf
	mov rdx, 1
	syscall
	mov rax, 0
	mov eax, [buf]
	ret

section .data
buf:
	dd 0
	dd 0
	dd 0
	dd 0
