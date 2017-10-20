LIBS = -lm
OPTS = -O3

all: objects link finish

objects:
	$(CC) $(OPTS) -c embedimage.c -o embedimage.o
	$(CC) $(OPTS) -c extend.c -o extend.o
	$(CC) $(OPTS) -c unu.c -o unu.o
	$(CC) $(OPTS) -c muri.c -o muri.o

link:
	$(CC) $(LIBS) unu.o -o unu
	$(CC) $(LIBS) muri.o -o muri
	$(CC) $(LIBS) embedimage.o -o embedimage
	$(CC) $(LIBS) extend.o -o extend

finish:
	mv embedimage ../bin
	mv extend ../bin
	mv unu ../bin
	mv muri ../bin
	rm *.o
