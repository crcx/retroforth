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
	./bin/unu literate/Nga.md >source/nga.c
	./bin/unu literate/Muri.md >tools/muri.c
	./bin/unu source/io/posix-files.forth   | ./bin/tanu posix_files >source/io/posix_files.c
	./bin/unu source/io/posix-args.forth    | ./bin/tanu posix_args >source/io/posix_args.c
	./bin/unu source/io/getc.forth          | ./bin/tanu posix_getc >source/io/getc.c
	./bin/unu source/io/FloatingPoint.forth | ./bin/tanu fpu >source/io/fpu.c
	./bin/unu source/io/gopher.forth        | ./bin/tanu gopher >source/io/gopher.c

	./bin/embedimage >source/image.c

image:
	./bin/muri literate/Rx.md
	./bin/extend literate/RetroForth.md

finally:
	cd source && make
