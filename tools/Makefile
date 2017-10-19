LIBS = -lm
OPTS = -O3

all: objects link finish

objects:
	$(CC) $(OPTS) -c embedimage.c -o embedimage.o
	$(CC) $(OPTS) -c unu.c -o unu.o
	$(CC) $(OPTS) -c muri.c -o muri.o
	$(CC) $(OPTS) -c tanu.c -o tanu.o

link:
	$(CC) $(LIBS) unu.o -o unu
	$(CC) $(LIBS) muri.o -o muri
	$(CC) $(LIBS) tanu.o -o tanu
	$(CC) $(LIBS) embedimage.o -o embedimage

finish:
	mv embedimage ../bin
	mv unu ../bin
	mv muri ../bin
	mv tanu ../bin
	rm *.o
