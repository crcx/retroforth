LIBS = -lm
OPTS = -Wall -O3

all: clean first then image finally

clean:
	rm -f bin/rre
	rm -f bin/embedimage
	rm -f bin/repl
	rm -f bin/extend
	rm -f bin/muri
	rm -f bin/tanu

first:
	cd tools && make

then:
	./bin/unu literate/Unu.md >tools/unu.c
	./bin/unu literate/Muri.md >tools/muri.c

image:
	./bin/muri literate/Rx.md
	./bin/extend literate/RetroForth.md
	cp ngaImage source
	cd source && ../bin/extend rre.forth
	cd source && ../bin/embedimage >image.c
	rm source/ngaImage

finally:
	cd source && make
