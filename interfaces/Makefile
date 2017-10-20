LIBS = -lm
OPTS = -O3

all: objects link finish

objects:
	$(CC) $(OPTS) -c rre.c -o rre.o
	$(CC) $(OPTS) -c repl.c -o repl.o

link:
	$(CC) $(LIBS) rre.o -o rre
	$(CC) $(LIBS) repl.o -o repl

finish:
	mv rre ../bin
	mv repl ../bin
	rm *.o
