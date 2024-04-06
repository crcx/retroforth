# This is the Makefile for Linux, macOS, and other systems
# using GNU Make. For BSD users, look at the Makefile instead.
# -------------------------------------------------------------

include Configuration.mk

ENABLED += -DNEEDS_STRL

LIBDL += -ldl

# -------------------------------------------------------------

all: clean build

help:

build: dirs toolchain ngaImage bin/retro bin/retro-describe

optional: build bin/retro-repl

toolchain: dirs bin/retro-embedimage bin/retro-extend bin/retro-muri bin/retro-unu

image: vm/nga-c/image.c

repl: bin/retro-repl

dirs:
	mkdir -p bin
	cp tools/retro-document.sh bin/retro-document

clean:
	rm -f bin/*


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
	install -m 755 -d -- $(DESTDIR)$(MANDIR)
	install -c -m 644 man/retro.1 $(DESTDIR)$(MANDIR)/retro.1
	install -c -m 644 man/retro-embedimage.1 $(DESTDIR)$(MANDIR)/retro-embedimage.1
	install -c -m 644 man/retro-extend.1 $(DESTDIR)$(MANDIR)/retro-extend.1
	install -c -m 644 man/retro-describe.1 $(DESTDIR)$(MANDIR)/retro-describe.1
	install -c -m 644 man/retro-document.1 $(DESTDIR)$(MANDIR)/retro-document.1
	install -c -m 644 man/retro-muri.1 $(DESTDIR)$(MANDIR)/retro-muri.1
	install -c -m 644 man/retro-unu.1 $(DESTDIR)$(MANDIR)/retro-unu.1
	install -c -m 644 man/retro-tags.1 $(DESTDIR)$(MANDIR)/retro-tags.1
	install -c -m 644 man/retro-locate.1 $(DESTDIR)$(MANDIR)/retro-locate.1



# toolchain targets

bin/retro-describe: tools/retro-describe.retro doc/words.tsv
	@cat tools/retro-describe.retro > bin/retro-describe
	@cat doc/words.tsv >> bin/retro-describe
	@chmod +x bin/retro-describe

bin/retro-embedimage: tools/retro-embedimage.c
	@$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o $@ $^

bin/retro-extend: tools/retro-extend.c
	@$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o $@ $^

bin/retro-muri: tools/retro-muri.c
	@$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o $@ $^

bin/retro-unu: tools/retro-unu.c
	@$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o $@ $^


# basic image target

ngaImage: toolchain image/retro.muri image/retro.forth image/build.retro
	@$(ASSEMBLE) image/retro.muri
	@$(EXTEND) ngaImage image/retro.forth image/build.retro

# minimal system

bin/retro-repl: vm/nga-c/repl.c vm/nga-c/image.c
	@$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o $@ $>


# retro on unix

update-extensions: bin/retro
	cd package/extensions && ../../bin/retro -f ../../tools/generate-extensions-list.retro >../load-extensions.retro

vm/nga-c/image.c: ngaImage bin/retro-embedimage bin/retro-extend $(DEVICES) interface/retro-unix.retro
	cp ngaImage rre.image
	./bin/retro-extend rre.image $(DEVICES) interface/retro-unix.retro
	./bin/retro-embedimage rre.image >vm/nga-c/image.c

bin/retro: vm/nga-c/image.c vm/nga-c/retro.c package/list.forth package/load-extensions.retro
	cd vm/nga-c && $(CC) $(OPTIONS) $(ENABLED) $(CFLAGS) $(LDFLAGS) -o ../../bin/retro retro.c $(LIBM) $(LIBDL)
	cd package && ../bin/retro -u rre.image -f list.forth
	./bin/retro-embedimage rre.image >vm/nga-c/image.c
	rm rre.image
	cd vm/nga-c && $(CC) $(OPTIONS) $(ENABLED) $(CFLAGS) $(LDFLAGS) -o ../../bin/retro retro.c $(LIBM) $(LIBDL)



# optional targets

bin/retro-compiler: toolchain vm/nga-c/retro-compiler.c vm/nga-c/retro-runtime.c
	@cp ngaImage runtime.image
	@$(EXTEND) runtime.image $(DEVICES) interface/retro-unix.retro
	@cd vm/nga-c && $(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o ../../retro-runtime retro-runtime.c $(LIBM) $(LIBDL)
	@cd vm/nga-c && $(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o ../../bin/retro-compiler retro-compiler.c
	@objcopy --add-section .ngaImage=runtime.image --set-section-flags .ngaImage=noload,readonly bin/retro-compiler
	@objcopy --add-section .runtime=retro-runtime --set-section-flags .runtime=noload,readonly bin/retro-compiler
	@rm runtime.image retro-runtime

image-js: bin/retro
	./bin/retro example/retro-generate-image-js.retro >vm/nga-js/image.js



# documentation targets

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



# other targets

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

