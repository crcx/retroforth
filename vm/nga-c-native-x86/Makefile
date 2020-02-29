all: native386 clean

native386:
	cp ../../ngaImage nativeImage
	../../bin/retro-extend nativeImage x86/common.retro x86/cmos-rtc.retro x86/serial.retro x86/display.retro x86/ata.retro x86/listener.retro
# x86/Block-Editor.retro
	../../bin/retro-embedimage nativeImage >image.c
	cc -fno-pie -Wall -m32 -DTARGET_X86              -c retro.c -o retro_qwerty.o
	cc -fno-pie -Wall -m32 -DTARGET_X86 -DUSE_DVORAK -c retro.c -o retro_dvorak.o
	nasm -f elf 386.s
	ld -z notext -nostdlib -m elf_i386 -T 386.ld 386.o retro_qwerty.o -o bin/rnf_qwerty.386
	ld -z notext -nostdlib -m elf_i386 -T 386.ld 386.o retro_dvorak.o -o bin/rnf_dvorak.386

clean:
	rm -f *.o
#	rm bin/rf.*
