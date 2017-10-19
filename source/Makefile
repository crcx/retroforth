RRE_FLAGS = -DFPU -DPOSIX_FILES -DPOSIX_ARGS -DARGV -DGOPHER
LIBS = -lm
OPTS = -O3

all: objects link finish

objects:
	$(CC) $(OPTS) -c bridge.c -o bridge.o
	$(CC) $(OPTS) -c rre.c -o rre.o
	$(CC) $(OPTS) -c repl.c -o repl.o
	$(CC) $(OPTS) -c nga.c -o nga.o
	$(CC) $(OPTS) -c image.c -o image.o

link:
	$(CC) $(LIBS) rre.o bridge.o nga.o image.o -o rre
	$(CC) $(LIBS) repl.o -o repl

finish:
	mv rre ../bin
	mv repl ../bin
	rm *.o
