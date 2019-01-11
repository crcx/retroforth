PREFIX ?= /usr/local
DATADIR ?= $(PREFIX)/share/RETRO12
DOCSDIR ?= $(PREFIX)/share/doc/RETRO12
EXAMPLESDIR ?= $(PREFIX)/share/examples/RETRO12
LIBM ?= -lm
LIBCURSES ?= -lcurses

all: build

build: dirs bin/retro-embedimage bin/retro-extend bin/retro-injectimage-js bin/retro-muri bin/RETRO12.html bin/retro-ri bin/retro bin/retro-repl bin/retro-unu bin/retro-compiler

dirs:
	mkdir -p bin

clean:
	rm -f bin/retro-embedimage
	rm -f bin/retro-extend
	rm -f bin/retro-injectimage-js
	rm -f bin/retro-muri
	rm -f bin/RETRO12.html
	rm -f bin/retro-repl
	rm -f bin/retro-ri
	rm -f bin/retro
	rm -f bin/retro-compiler
	rm -f bin/retro-unu
	rm -f interfaces/io/*.o

install: build install-data install-docs install-examples
	install -m 755 -d -- $(DESTDIR)$(PREFIX)/bin
	install -c -m 755 bin/retro-embedimage $(DESTDIR)$(PREFIX)/bin/retro-embedimage
	install -c -m 755 bin/retro-extend $(DESTDIR)$(PREFIX)/bin/retro-extend
	install -c -m 755 bin/retro-injectimage-js $(DESTDIR)$(PREFIX)/bin/retro-injectimage-js
	install -c -m 755 bin/retro-muri $(DESTDIR)$(PREFIX)/bin/retro-muri
	install -c -m 755 bin/retro-repl $(DESTDIR)$(PREFIX)/bin/retro-repl
	install -c -m 755 bin/retro-ri $(DESTDIR)$(PREFIX)/bin/retro-ri
	install -c -m 755 bin/retro $(DESTDIR)$(PREFIX)/bin/retro
	install -c -m 755 bin/retro-unu $(DESTDIR)$(PREFIX)/bin/retro-unu

install-strip: build install-data install-docs install-examples
	install -m 755 -d -- $(DESTDIR)/bin
	install -c -m 755 -s bin/retro-embedimage $(DESTDIR)$(PREFIX)/bin/retro-embedimage
	install -c -m 755 -s bin/retro-extend $(DESTDIR)$(PREFIX)/bin/retro-extend
	install -c -m 755 -s bin/retro-injectimage-js $(DESTDIR)$(PREFIX)/bin/retro-injectimage-js
	install -c -m 755 -s bin/retro-muri $(DESTDIR)$(PREFIX)/bin/retro-muri
	install -c -m 755 -s bin/retro-repl $(DESTDIR)$(PREFIX)/bin/retro-repl
	install -c -m 755 -s bin/retro-ri $(DESTDIR)$(PREFIX)/bin/retro-ri
	install -c -m 755 -s bin/retro $(DESTDIR)$(PREFIX)/bin/retro
	install -c -m 755 -s bin/retro-unu $(DESTDIR)$(PREFIX)/bin/retro-unu

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

test: bin/retro
	./bin/retro tests/test-core.forth

# Targets for development/interactive usage

glossary: doc/Glossary.txt

image: interfaces/image.c

js: bin/RETRO12.html

repl: bin/retro-repl

ri: bin/retro-ri

update: bin/retro-unu literate/Unu.md literate/Muri.md
	./bin/retro-unu literate/Unu.md >tools/unu.c
	./bin/retro-unu literate/Muri.md >tools/muri.c

# File targets.

bin/retro-embedimage: tools/embedimage.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/retro-embedimage  tools/embedimage.c

bin/retro-extend: tools/extend.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/retro-extend  tools/extend.c

bin/retro-injectimage-js: tools/injectimage-js.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/retro-injectimage-js  tools/injectimage-js.c

bin/retro-muri: tools/muri.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/retro-muri tools/muri.c

bin/RETRO12.html: bin/retro-injectimage-js
	./bin/retro-injectimage-js >bin/RETRO12.html

bin/retro-repl: interfaces/repl.c interfaces/image.c
	cd interfaces && $(CC) $(CFLAGS) $(LDFLAGS) -o ../bin/retro-repl repl.c

bin/retro-ri: io ioforth bin/retro-embedimage bin/retro-extend interfaces/image.c interfaces/ri.c interfaces/ri.forth
	cp ngaImage ri.image
	./bin/retro-extend ri.image interfaces/io/io_filesystem.forth
	./bin/retro-extend ri.image interfaces/io/io_gopher.forth
	./bin/retro-extend ri.image interfaces/io/io_floatingpoint.forth
	./bin/retro-extend ri.image interfaces/io/io_unix_syscalls.forth
	./bin/retro-extend ri.image interfaces/ri.forth
	./bin/retro-embedimage ri.image >interfaces/ri_image.c
	rm ri.image
	cd interfaces && $(CC) $(CFLAGS) $(LDFLAGS) -o ../bin/retro-ri $(LIBCURSES) $(LIBM) ri.c io/filesystem.o io/floatingpoint.o io/gopher.o io/unix.o

bin/retro: io ioforth bin/retro-embedimage bin/retro-extend interfaces/image.c interfaces/rre.c interfaces/rre.forth
	cp ngaImage rre.image
	./bin/retro-extend rre.image interfaces/io/io_filesystem.forth
	./bin/retro-extend rre.image interfaces/io/io_gopher.forth
	./bin/retro-extend rre.image interfaces/io/io_floatingpoint.forth
	./bin/retro-extend rre.image interfaces/io/io_unix_syscalls.forth
	./bin/retro-extend rre.image interfaces/rre.forth
	./bin/retro-embedimage rre.image >interfaces/rre_image.c
	rm rre.image
	cd interfaces && $(CC) $(CFLAGS) $(LDFLAGS) -o ../bin/retro $(LIBM) rre.c io/filesystem.o io/floatingpoint.o io/gopher.o io/unix.o

bin/retro-barebones: io ioforth bin/retro-embedimage bin/retro-extend interfaces/image.c interfaces/barebones.c interfaces/barebones.forth
	cp ngaImage barebones.image
	./bin/retro-extend barebones.image interfaces/barebones.forth
	./bin/retro-embedimage barebones.image >interfaces/barebones_image.c
	rm barebones.image
	cd interfaces && $(CC) $(CFLAGS) $(LDFLAGS) -o ../bin/retro-barebones barebones.c

bin/retro-unu: tools/unu.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o bin/retro-unu tools/unu.c

doc/Glossary.txt: bin/retro words.tsv
	LC_ALL=C sort -o sorted.tsv words.tsv
	mv sorted.tsv words.tsv
	./bin/retro glossary.forth export glossary >doc/Glossary.txt

interfaces/image.c: bin/retro-embedimage bin/retro-extend bin/retro-muri literate/RetroForth.md literate/Rx.md
	./bin/retro-muri literate/Rx.md
	./bin/retro-extend ngaImage literate/RetroForth.md
	./bin/retro-embedimage ngaImage > interfaces/image.c

bin/retro-compiler: io ioforth bin/retro-extend interfaces/image.c interfaces/retro-compiler.c interfaces/retro-compiler-runtime.c
	cp ngaImage runtime.image
	./bin/retro-extend runtime.image interfaces/io/io_filesystem.forth
	cd interfaces && $(CC) $(CFLAGS) $(LDFLAGS) -o ../retro-compiler-runtime $(LIBM) retro-compiler-runtime.c io/filesystem.o
	cd interfaces && $(CC) $(CFLAGS) $(LDFLAGS) -o ../bin/retro-compiler retro-compiler.c
	objcopy --add-section .ngaImage=runtime.image --set-section-flags .ngaImage=noload,readonly bin/retro-compiler
	objcopy --add-section .runtime=retro-compiler-runtime --set-section-flags .runtime=noload,readonly bin/retro-compiler
	rm runtime.image retro-compiler-runtime

ioforth: interfaces/io/io_filesystem.forth interfaces/io/io_floatingpoint.forth interfaces/io/io_gopher.forth interfaces/io/io_unix_syscalls.forth

io: interfaces/io/io_filesystem.c interfaces/io/io_floatingpoint.c interfaces/io/io_gopher.c interfaces/io/io_unix_syscalls.c
	cd interfaces/io && $(CC) $(CFLAGS) -c -o filesystem.o io_filesystem.c
	cd interfaces/io && $(CC) $(CFLAGS) -c -o floatingpoint.o io_floatingpoint.c
	cd interfaces/io && $(CC) $(CFLAGS) -c -o gopher.o io_gopher.c
	cd interfaces/io && $(CC) $(CFLAGS) -c -o unix.o io_unix_syscalls.c
