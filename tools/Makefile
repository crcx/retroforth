LIBS = -lm
OPTS = -O3

all: objects link finish

objects:
	$(CC) $(OPTS) -c embedimage.c -o embedimage.o
	$(CC) $(OPTS) -c extend.c -o extend.o
	$(CC) $(OPTS) -c unu.c -o unu.o
	$(CC) $(OPTS) -c muri.c -o muri.o

link:
	$(CC) unu.o        $(LIBS) -o unu
	$(CC) muri.o       $(LIBS) -o muri
	$(CC) embedimage.o $(LIBS) -o embedimage
	$(CC) extend.o     $(LIBS) -o extend

finish:
	mv embedimage ../bin
	mv extend ../bin
	mv unu ../bin
	mv muri ../bin
	rm *.o
