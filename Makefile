LIBS = -lm
OPTS = -Wall -O3

default: clean finally

all: clean first then image finally

clean:
	rm -f bin/rre
	rm -f bin/embedimage
	rm -f bin/repl
	rm -f bin/extend
	rm -f bin/muri

first:
	cd tools && make

then:
	./bin/unu literate/Unu.md >tools/unu.c
	./bin/unu literate/Muri.md >tools/muri.c

image:
	./bin/muri literate/Rx.md
	./bin/extend literate/RetroForth.md
	cp ngaImage interfaces
	cd interfaces && ../bin/extend rre.forth
	cd interfaces && ../bin/embedimage >image.c
	rm interfaces/ngaImage

finally:
	cd interfaces && make

