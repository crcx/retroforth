CFLAGS = -Wall -O3
default: clean toolchain update image repl rre glossary

clean:
	rm -f bin/unu
	rm -f bin/rre
	rm -f bin/embedimage
	rm -f bin/repl
	rm -f bin/extend
	rm -f bin/muri

toolchain:
	$(CC) $(CFLAGS) tools/embedimage.c -o bin/embedimage
	$(CC) $(CFLAGS) tools/extend.c -o bin/extend
	$(CC) $(CFLAGS) tools/unu.c -o bin/unu
	$(CC) $(CFLAGS) tools/muri.c -o bin/muri

update:
	./bin/unu literate/Unu.md >tools/unu.c
	./bin/unu literate/Muri.md >tools/muri.c

image:
	./bin/muri literate/Rx.md
	./bin/extend literate/RetroForth.md
	./bin/embedimage > interfaces/image.c

rre:
	cp ngaImage cleanImage
	./bin/extend interfaces/rre.forth
	./bin/embedimage >interfaces/rre_image_unix.c
	mv cleanImage ngaImage
	cp ngaImage cleanImage
	./bin/extend interfaces/rre_windows.forth
	./bin/embedimage >interfaces/rre_image_windows.c
	mv cleanImage ngaImage
	cd interfaces && $(CC) $(CFLAGS) -lm rre.c -o ../bin/rre

repl:
	cd interfaces && $(CC) $(CFLAGS) repl.c -o ../bin/repl

glossary:
	LC_ALL=c sort -o sorted.tsv words.tsv
	mv sorted.tsv words.tsv
	./bin/rre glossary.forth export glossary >doc/Glossary.txt
