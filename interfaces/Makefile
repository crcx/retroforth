LIBS = -lm
OPTS = -O3

all: objects link finish

objects:
	$(CC) $(OPTS) -c rre.c -o rre.o
	$(CC) $(OPTS) -c repl.c -o repl.o

link:
	$(CC) rre.o $(LIBS) -o rre
	$(CC) repl.o $(LIBS) -o repl

finish:
	mv rre ../bin
	mv repl ../bin
	rm *.o
