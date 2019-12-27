default:
	@echo specify the target:
	@echo -  freebsd-x86-32bit
	@echo -  freebsd-x86-64bit
	@echo -  macos
	@echo -  linux-x86-32bit

common:
	mkdir -p bin
	cp ../../ngaImage nativeImage
	../../bin/retro-embedimage nativeImage >image.c
	rm nativeImage

clean:
	rm *.o

freebsd-x86-32bit: common
	cc -m32 -c retro.c
	nasm -f elf fbsd.s
	ld -nostdlib -m elf_i386_fbsd fbsd.o retro.o -o bin/retro.freebsd32

freebsd-x86-64bit: common
	cc -m64 -c retro.c
	nasm -f elf64 fbsd64.s
	ld -nostdlib -m elf_x86_64_fbsd fbsd64.o retro.o -o bin/retro.freebsd64

macos: common
	cc -m64 -c retro.c
	nasm -f macho64 macos.s
	ld macos.o retro.o -o bin/retro.macos

linux-x86-32bit: common
	cc -m32 -c retro.c
	nasm -f elf linux.s
	ld -nostdlib linux.o retro.o -o bin/retro.linux32
