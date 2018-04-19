PREFIX = /usr/local

all: build bin/ri

build: bin/embedimage bin/extend bin/injectimage-js bin/muri bin/RETRO12.html bin/rre bin/repl bin/unu

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

install: build
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -p bin/embedimage $(DESTDIR)$(PREFIX)/bin/
	cp -p bin/extend $(DESTDIR)$(PREFIX)/bin/
	cp -p bin/injectimage-js $(DESTDIR)$(PREFIX)/bin/
	cp -p bin/muri $(DESTDIR)$(PREFIX)/bin/
	cp -p bin/repl $(DESTDIR)$(PREFIX)/bin/
	cp -p bin/rre $(DESTDIR)$(PREFIX)/bin/
	cp -p bin/unu $(DESTDIR)$(PREFIX)/bin/

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
	$(CC) $(CFLAGS) $(LDFLAGS) tools/embedimage.c -o bin/embedimage

bin/extend: tools/extend.c
	$(CC) $(CFLAGS) $(LDFLAGS) tools/extend.c -o bin/extend

bin/injectimage-js: tools/injectimage-js.c
	$(CC) $(CFLAGS) $(LDFLAGS) tools/injectimage-js.c -o bin/injectimage-js

bin/muri: tools/muri.c
	$(CC) $(CFLAGS) $(LDFLAGS) tools/muri.c -o bin/muri

bin/RETRO12.html: bin/injectimage-js
	./bin/injectimage-js >bin/RETRO12.html

bin/repl: interfaces/repl.c interfaces/image.c
	cd interfaces && $(CC) $(CFLAGS) $(LDFLAGS) repl.c -o ../bin/repl

bin/ri: interfaces/ri.c interfaces/image.c
	cd interfaces && $(CC) $(CFLAGS) $(LDFLAGS) -lcurses ri.c -o ../bin/ri

bin/rre: bin/embedimage bin/extend interfaces/image.c interfaces/rre.c interfaces/rre.forth
	cp ngaImage cleanImage
	./bin/extend interfaces/rre.forth
	./bin/embedimage >interfaces/rre_image_unix.c
	mv cleanImage ngaImage
	cd interfaces && $(CC) $(CFLAGS) $(LDFLAGS) -lm rre.c -o ../bin/rre

# XXX: I am not sure if the Windows build works.
rre_windows: bin/embedimage bin/extend interfaces/rre.c interface/image.c interfaces/rre_windows.forth
	cp ngaImage cleanImage
	./bin/extend interfaces/rre_windows.forth
	./bin/embedimage >interfaces/rre_image_windows.c
	mv cleanImage ngaImage

bin/unu: tools/unu.c
	$(CC) $(CFLAGS) $(LDFLAGS) tools/unu.c -o bin/unu

doc/Glossary.txt: bin/rre words.tsv
	LC_ALL=c sort -o sorted.tsv words.tsv
	mv sorted.tsv words.tsv
	./bin/rre glossary.forth export glossary >doc/Glossary.txt

interfaces/image.c: bin/embedimage bin/extend bin/muri literate/RetroForth.md literate/Rx.md
	./bin/muri literate/Rx.md
	./bin/extend literate/RetroForth.md
	./bin/embedimage > interfaces/image.c
