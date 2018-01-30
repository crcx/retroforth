section .multiboot_header
header_start:
    dd 0xe85250d6                ; magic number (multiboot 2)
    dd 0                         ; architecture 0 (protected mode i386)
    dd header_end - header_start ; header length
    ; checksum
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    ; insert optional multiboot tags here

    ; required end tag
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:


section .text
extern main
global _start
global putchar
global getchar

align 4
_start:
	mov dword [0xb8000], 0x2f4b2f4f
	jmp main
	jmp $

align 4
putchar:
	ret

align 4
getchar:
	ret
