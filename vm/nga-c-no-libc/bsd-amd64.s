.section ".note.openbsd.ident", "a"
  .p2align 2
  .long   0x8
  .long   0x4
  .long   0x1
  .ascii "OpenBSD\0"
  .long   0x0
   .p2align 2

.section .text
.global putchar
.global getchar
.global _start
.extern main

.p2align 8
_start:
  call    main
  movl   $0x0,%edi
  movq   $0x1,%rax
  syscall

putchar:
  mov    %rdi,%rax
  mov    %eax,buf
  mov    $0x4,%rax
  mov    $0x1,%rdi
  mov    $buf,%rsi
  mov    $0x1,%rdx
  syscall
  retq

getchar:
  mov    $0x3,%rax
  mov    $0x0,%rdi
  mov    $buf,%rsi
  mov    $0x1,%rdx
  syscall
  mov    $0x0,%rax
  mov    buf,%eax
  retq

.section .data
buf:
  .long 0
