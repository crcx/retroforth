;  ___ ___ _____ ___  ___      __  ___            ___ ___ ___
; | _ \ __|_   _| _ \/ _ \    / / | __| _ ___ ___| _ ) __|   \
; |   / _|  | | |   / (_) |  / /  | _| '_/ -_) -_) _ \__ \ |) |
; |_|_\___| |_| |_|_\\___/  /_/   |_||_| \___\___|___/___/___/
;
; This is the minimal startup + I/O functionality needed to run
; RETRO on a FreeBSD x86 system.
; =============================================================

bits 32
section .text
global putchar
global getchar
global _start
extern main

align 4
_start:
	call main
	jmp $

align 4
putchar:
	mov eax, [esp+4]
	mov [buf], eax
	push dword 1
	push dword buf
	push dword 1
	mov eax, 4
	call kernel
	add esp, 12
	ret

align 4
getchar:
	push dword 1
	push dword buf
	push dword 0
	mov eax, 3
	call kernel
	add esp, 12
	mov eax, 0
	mov eax, [buf]
	ret

align 4
kernel:
	int	80h
	ret

section .data
buf:
	dd 0
	dd 0
	dd 0
	dd 0
