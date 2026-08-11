# This is the Makefile for BSD systems. If using GNU Make (e.g.
# on Linux or macOS), look at the GNUmakefile instead.
# -------------------------------------------------------------

include Configuration.mk
VM_OBJECTS = \
	vm/nga-c/retro.o \
	vm/nga-c/image_data.o \
	vm/nga-c/nga_core.o \
	vm/nga-c/string_handling.o \
	vm/nga-c/scripting.o \
	vm/nga-c/dev-files.o \
	vm/nga-c/dev-float.o \
	vm/nga-c/dev-unix.o \
	vm/nga-c/dev-malloc.o \
	vm/nga-c/dev-blocks.o \
	vm/nga-c/dev-clock.o \
	vm/nga-c/dev-rng.o \
	vm/nga-c/dev-sockets.o \
	vm/nga-c/dev-multicore.o \
	vm/nga-c/dev-ffi.o \
	vm/nga-c/dev-error.o \
	vm/nga-c/dev-ioctl.o
VM_RUNTIME_SOURCES = \
	vm/nga-c/retro.c \
	vm/nga-c/nga_core.c \
	vm/nga-c/string_handling.c \
	vm/nga-c/scripting.c \
	vm/nga-c/dev-files.c \
	vm/nga-c/dev-float.c \
	vm/nga-c/dev-unix.c \
	vm/nga-c/dev-malloc.c \
	vm/nga-c/dev-blocks.c \
	vm/nga-c/dev-clock.c \
	vm/nga-c/dev-rng.c \
	vm/nga-c/dev-sockets.c \
	vm/nga-c/dev-multicore.c \
	vm/nga-c/dev-ffi.c \
	vm/nga-c/dev-error.c \
	vm/nga-c/dev-ioctl.c
.if $(PROFILE) == "full"
ENABLED += $(PROFILE_FULL)
.elif $(PROFILE) == "portable"
ENABLED += $(PROFILE_PORTABLE)
.elif $(PROFILE) == "minimal"
ENABLED += $(PROFILE_MINIMAL)
.else
.error Unknown PROFILE '$(PROFILE)' (expected full, portable, or minimal)
.endif

# -------------------------------------------------------------

all: build

help:

build: dirs toolchain ngaImage binaries

binaries: bin/retro bin/retro-repl bin/retro-describe

toolchain: dirs layout bin/retro-embedimage bin/retro-extend bin/retro-muri bin/retro-unu

image: vm/nga-c/image.c

dirs:
	@mkdir -p bin
	@cp tools/retro-document.sh bin/retro-document

clean:
	@rm -f bin/*
	@rm -f vm/nga-c/*.o

# installation targets

install: build install-data install-docs install-examples install-manpages
	install -m 755 -d -- $(DESTDIR)$(PREFIX)/bin
	install -c -m 755 bin/retro-embedimage $(DESTDIR)$(PREFIX)/bin/retro-embedimage
	install -c -m 755 bin/retro-extend $(DESTDIR)$(PREFIX)/bin/retro-extend
	install -c -m 755 bin/retro-muri $(DESTDIR)$(PREFIX)/bin/retro-muri
	install -c -m 755 bin/retro $(DESTDIR)$(PREFIX)/bin/retro
	install -c -m 755 bin/retro-unu $(DESTDIR)$(PREFIX)/bin/retro-unu
	install -c -m 755 bin/retro-describe $(DESTDIR)$(PREFIX)/bin/retro-describe
	install -c -m 755 bin/retro-document $(DESTDIR)$(PREFIX)/bin/retro-document
	install -c -m 755 example/retro-tags.retro $(DESTDIR)$(PREFIX)/bin/retro-tags
	install -c -m 755 example/retro-locate.retro $(DESTDIR)$(PREFIX)/bin/retro-locate

install-strip: build install-data install-docs install-examples
	install -m 755 -d -- $(DESTDIR)/bin
	install -c -m 755 -s bin/retro-embedimage $(DESTDIR)$(PREFIX)/bin/retro-embedimage
	install -c -m 755 -s bin/retro-extend $(DESTDIR)$(PREFIX)/bin/retro-extend
	install -c -m 755 -s bin/retro-muri $(DESTDIR)$(PREFIX)/bin/retro-muri
	install -c -m 755 -s bin/retro $(DESTDIR)$(PREFIX)/bin/retro
	install -c -m 755 -s bin/retro-unu $(DESTDIR)$(PREFIX)/bin/retro-unu
	install -c -m 755 bin/retro-describe $(DESTDIR)$(PREFIX)/bin/retro-describe
	install -c -m 755 bin/retro-document $(DESTDIR)$(PREFIX)/bin/retro-document
	install -c -m 755 example/retro-tags.retro $(DESTDIR)$(PREFIX)/bin/retro-tags
	install -c -m 755 example/retro-locate.retro $(DESTDIR)$(PREFIX)/bin/retro-locate

install-data:
	install -m 755 -d -- $(DESTDIR)$(DATADIR)
	install -c -m 644 tools/glossary.retro $(DESTDIR)$(DATADIR)/glossary.retro
	install -c -m 644 ngaImage $(DESTDIR)$(DATADIR)/ngaImage
	cp -fpR tests $(DESTDIR)$(DATADIR)/
	install -c -m 644 doc/words.tsv $(DESTDIR)$(DATADIR)/words.tsv

install-docs:
	install -m 755 -d -- $(DESTDIR)$(DOCSDIR)
	install -c -m 644 README.md $(DESTDIR)$(DOCSDIR)/README.md
	install -c -m 644 RELEASE-NOTES $(DESTDIR)$(DOCSDIR)/RELEASE-NOTES
	install -c -m 644 doc/RETRO-Book.md $(DESTDIR)$(DOCSDIR)/RETRO-Book.md
	install -c -m 644 doc/words.tsv $(DESTDIR)$(DOCSDIR)/words.tsv

install-examples:
	install -m 755 -d -- $(DESTDIR)$(EXAMPLESDIR)
	cp -fpR example $(DESTDIR)$(EXAMPLESDIR)

install-manpages:
	install -m 755 -d -- $(MANDIR)
	install -c -m 644 man/retro.1 $(MANDIR)/retro.1
	install -c -m 644 man/retro-embedimage.1 $(MANDIR)/retro-embedimage.1
	install -c -m 644 man/retro-extend.1 $(MANDIR)/retro-extend.1
	install -c -m 644 man/retro-describe.1 $(MANDIR)/retro-describe.1
	install -c -m 644 man/retro-document.1 $(MANDIR)/retro-document.1
	install -c -m 644 man/retro-muri.1 $(MANDIR)/retro-muri.1
	install -c -m 644 man/retro-unu.1 $(MANDIR)/retro-unu.1
	install -c -m 644 man/retro-tags.1 $(MANDIR)/retro-tags.1
	install -c -m 644 man/retro-locate.1 $(MANDIR)/retro-locate.1

# Toolchain ----------------------------------------------------

layout: dirs bin/generate-layout tools/layout/image.tsv tools/layout/dictionary.tsv
	@mkdir -p tools/generated
	@./bin/generate-layout

bin/generate-layout: tools/generate-layout.c
	@$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o $@ $>

bin/retro-describe: tools/retro-describe.retro
	@cp tools/retro-describe.retro bin/retro-describe
#	@cat tools/retro-describe.retro > bin/retro-describe
#	@cat doc/words.tsv >> bin/retro-describe
#	@chmod +x bin/retro-describe

bin/retro-embedimage: tools/retro-embedimage.c
	@$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o $@ $>

bin/retro-extend: tools/retro-extend.c
	@$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o $@ $>

bin/retro-muri: tools/retro-muri.c
	@$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o $@ $>

bin/retro-unu: tools/retro-unu.c
	@$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o $@ $>

# Image --------------------------------------------------------

ngaImage: bin/retro-muri bin/retro-extend image/retro.muri image/retro.forth image/build.retro
	@$(ASSEMBLE) image/retro.muri
	@$(EXTEND) ngaImage image/retro.forth image/build.retro

# Executables --------------------------------------------------

bin/retro-repl: vm/nga-c/repl.c vm/nga-c/image.c
	@$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o $@ $>

# retro on unix

update-extensions: bin/retro
	@cd package/extensions && ../../bin/retro -f ../../tools/generate-extensions-list.retro >../load-extensions.retro

bin/retro-runtime: $(VM_RUNTIME_SOURCES)
	@$(CC) -DNO_EMBEDDED_IMAGE -DFAST $(OPTIONS) $(ENABLED) $(CFLAGS) $(LDFLAGS) -o $@ $(VM_RUNTIME_SOURCES) $(LIBM) $(LIBDL)

bin/rre.image: ngaImage bin/retro-extend bin/retro-runtime interface/retro-unix.retro $(DEVICES) package/list.forth package/load-extensions.retro
	@cp ngaImage bin/rre.image
	@$(EXTEND) bin/rre.image $(DEVICES) interface/retro-unix.retro
	@cd package && ../bin/retro-runtime -u ../bin/rre.image -f list.forth

vm/nga-c/image.c: bin/rre.image bin/retro-embedimage
	@$(EXPORT) bin/rre.image >vm/nga-c/image.c

$(VM_OBJECTS): vm/nga-c/retro.h vm/nga-c/nga_core.h vm/nga-c/config.h vm/nga-c/devices.h vm/nga-c/devices.def

vm/nga-c/image_data.o: vm/nga-c/image.c

.c.o:
	$(CC) $(OPTIONS) $(ENABLED) $(CFLAGS) -c -o $@ $<

bin/retro: vm/nga-c/image.c $(VM_OBJECTS)
	$(CC) $(OPTIONS) $(ENABLED) $(CFLAGS) $(LDFLAGS) -o $@ $(VM_OBJECTS) $(LIBM) $(LIBDL)


# optional targets

image-js: bin/retro
	@./bin/retro example/retro-generate-image-js.retro >vm/nga-js/image.js

# Documentation ------------------------------------------------

glossary: doc/Glossary.txt doc/Glossary.html doc/Glossary-Concise.txt doc/Glossary-Names-and-Stack.txt doc/words.tsv

sorted: doc/words.tsv
	LC_ALL=C sort -o sorted.tsv doc/words.tsv
	mv sorted.tsv doc/words.tsv

doc/Glossary.txt: bin/retro sorted
	$(GLOSSARY) export glossary >doc/Glossary.txt

doc/Glossary.html: bin/retro sorted
	$(GLOSSARY) export html >doc/Glossary.html

doc/Glossary-Concise.txt: bin/retro sorted
	$(GLOSSARY) export concise >doc/Glossary-Concise.txt

doc/Glossary-Names-and-Stack.txt: bin/retro sorted
	$(GLOSSARY) export concise-stack >doc/Glossary-Names-and-Stack.txt

# Other Targets ------------------------------------------------

release: clean build glossary
	fossil tarball tip R12.tar.gz
	mkdir release
	cd release && tar xzvf ../R12.tar.gz
	cd release && mv * RETRO12-$(VERSION)
	cd release && tar cf RETRO12-$(VERSION).tar RETRO12-$(VERSION)
	cd release && gzip RETRO12-$(VERSION).tar
	mv release/RETRO12-$(VERSION).tar.gz .
	rm -rf release
	signify -S -s /home/crc/keys/$(KEYPAIR).sec -m RETRO12-$(VERSION).tar.gz
	signify -V -p security/$(KEYPAIR).pub -m RETRO12-$(VERSION).tar.gz

test: bin/retro
	./bin/retro tests/test-core.forth

update: bin/retro image/retro.forth image/retro.muri
	./bin/retro tools/update-build.retro > image/build.retro
