all: native386 clean

native386:
	cp ../../ngaImage nativeImage
	../../bin/retro-extend nativeImage x86/common.forth x86/cmos-rtc.forth x86/serial.forth x86/display.forth
	../../bin/retro-embedimage nativeImage >image.c
	cc -fno-pie -m32 -DTARGET_X86              -c retro.c -o retro_qwerty.o
	cc -fno-pie -m32 -DTARGET_X86 -DUSE_DVORAK -c retro.c -o retro_dvorak.o
	nasm -f elf 386.s
	ld -nostdlib -m elf_i386 -n -T 386.ld 386.o retro_qwerty.o -o bin/rnf_qwerty.386
	ld -nostdlib -m elf_i386 -n -T 386.ld 386.o retro_dvorak.o -o bin/rnf_dvorak.386
#	ld -nostdlib -m elf_i386 -n -e_start -T 386flat.ld 386.o retro.o -o bin/rf.386flat

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


clean:
	rm -f *.o
#	rm bin/rf.*
