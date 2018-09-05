PREFIX ?= /usr/local
DATADIR ?= $(PREFIX)/share/RETRO12
DOCSDIR ?= $(PREFIX)/share/doc/RETRO12
EXAMPLESDIR ?= $(PREFIX)/share/examples/RETRO12
LIBM ?= -lm
LIBCURSES ?= -lcurses

all: build

build: bin/embedimage bin/extend bin/injectimage-js bin/muri bin/RETRO12.html bin/ri bin/rre bin/repl bin/unu

clean:
	rm -f bin/embedimage
	rm -f bin/extend
	rm -f bin/injectimage-js
	rm -f bin/muri
	rm -f bin/RETRO12.html
	rm -f bin/repl
	rm -f bin/ri
	rm -f bin/rre
	rm -f bin/unu

install: build install-data install-docs install-examples
	install -m 755 -d -- $(DESTDIR)$(PREFIX)/bin
	install -c -m 755 bin/embedimage $(DESTDIR)$(PREFIX)/bin/embedimage
	install -c -m 755 bin/extend $(DESTDIR)$(PREFIX)/bin/extend
	install -c -m 755 bin/injectimage-js $(DESTDIR)$(PREFIX)/bin/injectimage-js
	install -c -m 755 bin/listener $(DESTDIR)$(PREFIX)/bin/listener
	install -c -m 755 bin/muri $(DESTDIR)$(PREFIX)/bin/muri
	install -c -m 755 bin/repl $(DESTDIR)$(PREFIX)/bin/repl
	install -c -m 755 bin/ri $(DESTDIR)$(PREFIX)/bin/ri
	install -c -m 755 bin/rre $(DESTDIR)$(PREFIX)/bin/rre
	install -c -m 755 bin/unu $(DESTDIR)$(PREFIX)/bin/unu

install-strip: build install-data install-docs install-examples
	install -m 755 -d -- $(DESTDIR)/bin
	install -c -m 755 -s bin/embedimage $(DESTDIR)$(PREFIX)/bin/embedimage
	install -c -m 755 -s bin/extend $(DESTDIR)$(PREFIX)/bin/extend
	install -c -m 755 -s bin/injectimage-js $(DESTDIR)$(PREFIX)/bin/injectimage-js
	install -c -m 755 bin/listener $(DESTDIR)$(PREFIX)/bin/listener
	install -c -m 755 -s bin/muri $(DESTDIR)$(PREFIX)/bin/muri
	install -c -m 755 -s bin/repl $(DESTDIR)$(PREFIX)/bin/repl
	install -c -m 755 -s bin/ri $(DESTDIR)$(PREFIX)/bin/ri
	install -c -m 755 -s bin/rre $(DESTDIR)$(PREFIX)/bin/rre
	install -c -m 755 -s bin/unu $(DESTDIR)$(PREFIX)/bin/unu

install-data: bin/RETRO12.html
	install -m 755 -d -- $(DESTDIR)$(DATADIR)
	install -c -m 644 bin/RETRO12.html $(DESTDIR)$(DATADIR)/RETRO12.html
	install -c -m 644 glossary.forth $(DESTDIR)$(DATADIR)/glossary.forth
	install -c -m 644 ngaImage $(DESTDIR)$(DATADIR)/ngaImage
	cp -fpR tests $(DESTDIR)$(DATADIR)/
	install -c -m 644 words.tsv $(DESTDIR)$(DATADIR)/words.tsv

install-docs:
	install -m 755 -d -- $(DESTDIR)$(DOCSDIR)
	cp -fpR doc $(DESTDIR)$(DOCSDIR)
	cp -fpR literate $(DESTDIR)$(DOCSDIR)
	install -c -m 644 README.md $(DESTDIR)$(DOCSDIR)/README.md
	install -c -m 644 RELEASE_NOTES.md $(DESTDIR)$(DOCSDIR)/RELEASE_NOTES.md

install-examples:
	install -m 755 -d -- $(DESTDIR)$(EXAMPLESDIR)
	cp -fpR example $(DESTDIR)$(EXAMPLESDIR)

test: bin/rre
	./bin/rre tests/test-core.forth

# Targets for development/interactive usage

glossary: doc/Glossary.txt

image: interfaces/image.c

js: bin/RETRO12.html

repl: bin/repl

ri: bin/ri

update: bin/unu literate/Unu.md literate/Muri.md
	./bin/unu literate/Unu.md >tools/unu.c
	./bin/unu literate/Muri.md >tools/muri.c

# File targets.

bin/embedimage: tools/embedimage.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/embedimage  tools/embedimage.c

bin/extend: tools/extend.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/extend  tools/extend.c

bin/injectimage-js: tools/injectimage-js.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/injectimage-js  tools/injectimage-js.c

bin/muri: tools/muri.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/muri tools/muri.c

bin/RETRO12.html: bin/injectimage-js
	./bin/injectimage-js >bin/RETRO12.html

bin/repl: interfaces/repl.c interfaces/image.c
	cd interfaces && $(CC) $(CFLAGS) $(LDFLAGS) -o ../bin/repl repl.c

bin/ri: interfaces/ri.c interfaces/image.c
	cd interfaces && $(CC) $(CFLAGS) $(LDFLAGS) -o ../bin/ri $(LIBCURSES) ri.c

bin/rre: bin/embedimage bin/extend interfaces/image.c interfaces/rre.c interfaces/rre.forth
	cp ngaImage cleanImage
	./bin/extend interfaces/rre.forth
	./bin/embedimage >interfaces/rre_image_unix.c
	mv cleanImage ngaImage
	cd interfaces && $(CC) $(CFLAGS) $(LDFLAGS) -o ../bin/rre $(LIBM) rre.c

bin/unu: tools/unu.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/unu tools/unu.c

doc/Glossary.txt: bin/rre words.tsv
	LC_ALL=C sort -o sorted.tsv words.tsv
	mv sorted.tsv words.tsv
	./bin/rre glossary.forth export glossary >doc/Glossary.txt

interfaces/image.c: bin/embedimage bin/extend bin/muri literate/RetroForth.md literate/Rx.md
	./bin/muri literate/Rx.md
	./bin/extend literate/RetroForth.md
	./bin/embedimage > interfaces/image.c
