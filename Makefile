PREFIX ?= /usr/local
DATADIR ?= $(PREFIX)/share/RETRO12
DOCSDIR ?= $(PREFIX)/share/doc/RETRO12
EXAMPLESDIR ?= $(PREFIX)/share/examples/RETRO12
MANDIR ?= $(PREFIX)/man/man1
LIBM ?= -lm

all: build

help:

build: dirs toolchain ngaImage bin/retro bin/retro-describe

optional: build bin/retro-repl

toolchain: dirs bin/retro-embedimage bin/retro-extend bin/retro-muri bin/retro-unu

dirs:
	mkdir -p bin
	cp tools/document.sh bin/retro-document

clean:
	rm -f bin/*

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
	cp -fpR doc $(DESTDIR)$(DOCSDIR)
#	cp -fpR literate $(DESTDIR)$(DOCSDIR)
	install -c -m 644 README $(DESTDIR)$(DOCSDIR)/README
	install -c -m 644 RELEASE-NOTES $(DESTDIR)$(DOCSDIR)/RELEASE-NOTES

install-examples:
	install -m 755 -d -- $(DESTDIR)$(EXAMPLESDIR)
	cp -fpR example $(DESTDIR)$(EXAMPLESDIR)

install-manpages:
	install -c -m 644 man/retro.1 $(MANDIR)/retro.1
	install -c -m 644 man/retro-embedimage.1 $(MANDIR)/retro-embedimage.1
	install -c -m 644 man/retro-extend.1 $(MANDIR)/retro-extend.1
	install -c -m 644 man/retro-describe.1 $(MANDIR)/retro-describe.1
	install -c -m 644 man/retro-document.1 $(MANDIR)/retro-document.1
	install -c -m 644 man/retro-muri.1 $(MANDIR)/retro-muri.1
	install -c -m 644 man/retro-unu.1 $(MANDIR)/retro-unu.1
	install -c -m 644 man/retro-tags.1 $(MANDIR)/retro-tags.1

test: bin/retro
	./bin/retro tests/test-core.forth

# Targets for development/interactive usage

glossary: doc/Glossary.txt doc/Glossary.html doc/Glossary-Concise.txt doc/Glossary-Names-and-Stack.txt doc/words.tsv

image: vm/nga-c/image.c

repl: bin/retro-repl

retro-describe: bin/retro-describe

# File targets.

ngaImage: image/rx.muri image/retro.forth bin/retro-muri bin/retro-extend
	./bin/retro-muri image/rx.muri
	./bin/retro-extend ngaImage image/retro.forth

bin/retro-describe: tools/retro-describe.retro doc/words.tsv
	cat tools/retro-describe.retro doc/words.tsv >bin/retro-describe
	chmod +x bin/retro-describe

bin/retro-embedimage: tools/embedimage.c
	$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o bin/retro-embedimage  tools/embedimage.c

bin/retro-extend: tools/extend.c
	$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o bin/retro-extend  tools/extend.c

bin/retro-injectimage-js: tools/injectimage-js.c
	$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o bin/retro-injectimage-js  tools/injectimage-js.c

bin/retro-muri: tools/muri.c
	$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o bin/retro-muri tools/muri.c

bin/retro-repl: vm/nga-c/repl.c vm/nga-c/image.c
	cd vm/nga-c && $(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o ../../bin/retro-repl repl.c

bin/retro: ngaImage bin/retro-embedimage bin/retro-extend vm/nga-c/retro-image.c vm/nga-c/retro-unix.c interface/filesystem.retro interface/floatingpoint.retro interface/unix.retro interface/rng.retro interface/sockets.retro interface/retro-unix.retro interface/clock.retro
	cp ngaImage rre.image
	./bin/retro-extend rre.image interface/filesystem.retro interface/floatingpoint.retro interface/unix.retro interface/rng.retro interface/sockets.retro interface/retro-unix.retro interface/clock.retro 
	./bin/retro-embedimage rre.image >vm/nga-c/retro-image.c
	cd vm/nga-c && $(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o ../../bin/retro retro-unix.c $(LIBM)
	cd package && ../bin/retro -f list.forth
	./bin/retro-embedimage rre.image >vm/nga-c/retro-image.c
	rm rre.image
	cd vm/nga-c && $(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o ../../bin/retro retro-unix.c $(LIBM)

bin/retro-unu: tools/unu.c
	$(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o bin/retro-unu tools/unu.c

sorted: doc/words.tsv
	LC_ALL=C sort -o sorted.tsv doc/words.tsv
	mv sorted.tsv doc/words.tsv

doc/Glossary.txt: bin/retro sorted
	./bin/retro tools/glossary.retro export glossary >doc/Glossary.txt

doc/Glossary.html: bin/retro sorted
	./bin/retro tools/glossary.retro export html >doc/Glossary.html

doc/Glossary-Concise.txt: bin/retro sorted
	./bin/retro tools/glossary.retro export concise >doc/Glossary-Concise.txt

doc/Glossary-Names-and-Stack.txt: bin/retro sorted
	./bin/retro tools/glossary.retro export concise-stack >doc/Glossary-Names-and-Stack.txt

vm/nga-c/image.c: bin/retro-embedimage bin/retro-extend bin/retro-muri image/retro.forth image/rx.muri
	./bin/retro-muri image/rx.muri
	./bin/retro-extend ngaImage image/retro.forth
	./bin/retro-embedimage ngaImage > vm/nga-c/image.c

bin/retro-compiler: bin/retro-extend vm/nga-c/retro-compiler.c vm/nga-c/retro-runtime.c
	cp ngaImage runtime.image
	./bin/retro-extend runtime.image interface/filesystem.retro interface/floatingpoint.retro interface/unix.retro interface/rng.retro interface/retro-unix.retro interface/clock.retro
	cd vm/nga-c && $(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o ../../retro-runtime retro-runtime.c $(LIBM)
	cd vm/nga-c && $(CC) $(OPTIONS) $(CFLAGS) $(LDFLAGS) -o ../../bin/retro-compiler retro-compiler.c
	objcopy --add-section .ngaImage=runtime.image --set-section-flags .ngaImage=noload,readonly bin/retro-compiler
	objcopy --add-section .runtime=retro-runtime --set-section-flags .runtime=noload,readonly bin/retro-compiler
	rm runtime.image retro-runtime
