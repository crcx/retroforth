#SRC: extend
#SRC: nga
#SRC: bridge
EXTENDLIBS = 
EXTENDFLAGS = 
extend:
	clang -c extend.c $(EXTENDFLAGS) -o extend.o
	clang -c nga.c $(EXTENDFLAGS) -o nga.o
	clang -c bridge.c $(EXTENDFLAGS) -o bridge.o
	clang $(EXTENDLIBS) extend.o nga.o bridge.o  -o extend
	mv extend ../bin
#SRC: embedimage
EMBEDIMAGELIBS = 
EMBEDIMAGEFLAGS = 
embedimage:
	clang -c embedimage.c $(EMBEDIMAGEFLAGS) -o embedimage.o
	clang $(EMBEDIMAGELIBS) embedimage.o  -o embedimage
	mv embedimage ../bin
#SRC: rre
#LIB: m
#SRC: nga
#SRC: bridge
#SRC: image
#FLG:  -Wall
#FLG:  -O3
#FLG:  -DFPU
#FLG:  -DPOSIX_FILES
#FLG:  -DPOSIX_ARGS
#FLG:  -DARGV
#FLG:  -DGOPHER
RRELIBS = -lm 
RREFLAGS =  -Wall  -O3  -DFPU  -DPOSIX_FILES  -DPOSIX_ARGS  -DARGV  -DGOPHER 
rre:
	clang -c rre.c $(RREFLAGS) -o rre.o
	clang -c nga.c $(RREFLAGS) -o nga.o
	clang -c bridge.c $(RREFLAGS) -o bridge.o
	clang -c image.c $(RREFLAGS) -o image.o
	clang $(RRELIBS) rre.o nga.o bridge.o image.o  -o rre
	mv rre ../bin
#SRC: repl
#SRC: nga
#SRC: bridge
REPLLIBS = 
REPLFLAGS = 
repl:
	clang -c repl.c $(REPLFLAGS) -o repl.o
	clang -c nga.c $(REPLFLAGS) -o nga.o
	clang -c bridge.c $(REPLFLAGS) -o bridge.o
	clang $(REPLLIBS) repl.o nga.o bridge.o  -o repl
	mv repl ../bin
