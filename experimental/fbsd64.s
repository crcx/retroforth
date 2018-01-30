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
