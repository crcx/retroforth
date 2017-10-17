#  ____   ____ ______ ____    ___
#  || \\ ||    | || | || \\  // \\
#  ||_// ||==    ||   ||_// ((   ))
#  || \\ ||___   ||   || \\  \\_//
#  a personal, minimalistic forth

CC = clang
LD = clang
LDFLAGS = -lm
CFLAGS = -Wall -O3

all: clean tools update_sources image interfaces finally
#test

clean:
	rm -f bin/rre bin/nga bin/embedimage bin/extend bin/unu bin/muri bin/kanga bin/repl bin/tanu bin/build

tools:
	cd source && $(CC) $(CFLAGS) unu.c -o ../bin/unu
	cd source && $(CC) $(CFLAGS) muri.c -o ../bin/muri
	cd source && $(CC) $(CFLAGS) tanu.c -o ../bin/tanu
	cd source && $(CC) $(CFLAGS) build.c -o ../bin/build
	cd source && make extend
	cd source && make embedimage

update_sources:
	./bin/unu literate/Unu.md >source/unu.c
	./bin/unu literate/Nga.md >source/nga.c
	./bin/unu literate/Muri.md >source/muri.c
	./bin/unu source/io/posix-files.forth   | ./bin/tanu posix_files >source/io/posix_files.c
	./bin/unu source/io/posix-args.forth    | ./bin/tanu posix_args >source/io/posix_args.c
	./bin/unu source/io/getc.forth          | ./bin/tanu posix_getc >source/io/getc.c
	./bin/unu source/io/FloatingPoint.forth | ./bin/tanu fpu >source/io/fpu.c
	./bin/unu source/io/gopher.forth        | ./bin/tanu gopher >source/io/gopher.c

	./bin/embedimage >source/image.c

image:
	./bin/muri literate/Rx.md
	./bin/extend literate/RetroForth.md

interfaces:
	cd source && make rre

finally:
	rm source/*.o

test:
	./bin/rre test-core.forth
