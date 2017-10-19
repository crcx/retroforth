RRE_FLAGS = -DFPU -DPOSIX_FILES -DPOSIX_ARGS -DARGV -DGOPHER
LIBS = -lm
OPTS = -O3

all: objects link finish

essentials: objects
	$(CC) $(LIBS) extend.o bridge.o nga.o -o extend
	$(CC) $(LIBS) unu.o -o unu
	$(CC) $(LIBS) muri.o -o muri
	$(CC) $(LIBS) tanu.o -o tanu
	$(CC) $(LIBS) embedimage.o -o embedimage
	mv extend ../bin
	mv embedimage ../bin
	mv unu ../bin
	mv muri ../bin
	mv tanu ../bin
	rm *.o

objects:
	$(CC) $(OPTS) -c embedimage.c -o embedimage.o
	$(CC) $(OPTS) -c unu.c -o unu.o
	$(CC) $(OPTS) -c muri.c -o muri.o
	$(CC) $(OPTS) -c tanu.c -o tanu.o
	$(CC) $(OPTS) -c bridge.c -o bridge.o
	$(CC) $(OPTS) -c extend.c -o extend.o
	$(CC) $(OPTS) -c rre.c -o rre.o
	$(CC) $(OPTS) -c repl.c -o repl.o
	$(CC) $(OPTS) -c nga.c -o nga.o
	$(CC) $(OPTS) -c image.c -o image.o

link:
	$(CC) $(LIBS) rre.o bridge.o nga.o image.o -o rre
	$(CC) $(LIBS) repl.o bridge.o nga.o -o repl

finish:
	mv rre ../bin
	mv repl ../bin
	rm *.o
