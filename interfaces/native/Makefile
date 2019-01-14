all: freebsd freebsd64 linux native386 clean

macos:
	cc -m64 -c retro.c
	nasm -f macho64 macos.s
	ld macos.o retro.o -o bin/rf.macos

freebsd:
	cc -m32 -c retro.c
	nasm -f elf fbsd.s
	ld -nostdlib -m elf_i386_fbsd fbsd.o retro.o -o bin/rf.bsd

freebsd64:
	cc -m64 -c retro.c
	nasm -f elf64 fbsd64.s
	ld -nostdlib -m elf_x86_64_fbsd fbsd64.o retro.o -o bin/rf.bsd64

openbsd:
	cc -m64 -fno-pie -fno-pic -c retro.c
	nasm -f elf64 obsd64.s
	cc -fno-pie -fno-pic -e _start -static obsd64.o retro.o -o bin/rf.bsd64

linux:
	cc -m32 -c retro.c
	nasm -f elf linux.s
	ld -nostdlib -m elf_i386_fbsd linux.o retro.o -o bin/rf.linux
#	brandelf -t Linux bin/rf.linux

native386:
	cc -fno-pie -m32 -c retro.c
	nasm -f elf 386.s
	ld -nostdlib -m elf_i386 -n -T 386.ld 386.o retro.o -o bin/rf.386
	ld -nostdlib -m elf_i386 -n -e_start -T 386flat.ld 386.o retro.o -o bin/rf.386flat

clean:
	rm -f *.o
#	rm bin/rf.*
