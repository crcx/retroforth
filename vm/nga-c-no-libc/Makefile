default:
	@echo specify the target:
	@echo -  freebsd-i386
	@echo -  freebsd-amd64
	@echo -  linux-i386
	@echo -  macos-amd64
	@echo -  openbsd-amd64

common:
	mkdir -p bin
	cp ../../ngaImage nativeImage
	../../bin/retro-embedimage nativeImage >image.c
	rm nativeImage

clean:
	rm *.o

bsd-i386:
	nasm -f elf bsd-i386.s

bsd-amd64.o: bsd-amd64.s
	cc -c bsd-amd64.s -o bsd-amd64.o

freebsd-i386: common bsd-i386
	cc -m32 -c retro.c
	ld -nostdlib -m elf_i386_fbsd bsd-i386.o retro.o -o bin/retro

freebsd-amd64: common bsd-amd64.o
	cc -m64 -c retro.c
	ld -nostdlib -no-pie -m elf_x86_64_fbsd bsd-amd64.o retro.o -o bin/retro

openbsd-amd64: common bsd-amd64.o
	cc -m64 -c retro.c
	ld -nostdlib -no-pie -m elf_x86_64 bsd-amd64.o retro.o -o bin/retro

macos-amd64: common
	cc -m64 -fno-pie -c retro.c
	nasm -f macho64 macos.s
	ld macos.o retro.o -o bin/retro

linux-i386: common
	cc -m32 -c retro.c
	nasm -f elf linux.s
	ld -nostdlib linux.o retro.o -o bin/retro
