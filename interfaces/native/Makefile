all: native386 clean

native386:
	cp ../../ngaImage nativeImage
	../../bin/retro-extend nativeImage x86/common.forth x86/keyboard.forth x86/cmos-rtc.forth x86/serial.forth x86/display.forth x86/ata.forth x86/listener.forth x86/Block-Editor.forth
	../../bin/retro-embedimage nativeImage >image.c
	cc -fno-pie -Wall -m32 -DTARGET_X86              -c retro.c -o retro_qwerty.o
	cc -fno-pie -Wall -m32 -DTARGET_X86 -DUSE_DVORAK -c retro.c -o retro_dvorak.o
	nasm -f elf 386.s
	ld -nostdlib -m elf_i386 -n -T 386.ld 386.o retro_qwerty.o -o bin/rnf_qwerty.386
	ld -nostdlib -m elf_i386 -n -T 386.ld 386.o retro_dvorak.o -o bin/rnf_dvorak.386

clean:
	rm -f *.o
#	rm bin/rf.*
