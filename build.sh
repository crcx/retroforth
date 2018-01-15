#!/bin/sh

rm -f bin/rre
rm -f bin/embedimage
rm -f bin/repl
rm -f bin/extend
rm -f bin/muri

cd tools
cc -O3 -c embedimage.c -o embedimage.o
cc -O3 -c extend.c -o extend.o
cc -O3 -c unu.c -o unu.o
cc -O3 -c muri.c -o muri.o
cc unu.o        -lm -o unu
cc muri.o       -lm -o muri
cc embedimage.o -lm -o embedimage
cc extend.o     -lm -o extend
mv embedimage ../bin
mv extend ../bin
mv unu ../bin
mv muri ../bin
rm *.o
cd ..

./bin/unu literate/Unu.md >tools/unu.c
./bin/unu literate/Muri.md >tools/muri.c
./bin/muri literate/Rx.md
./bin/extend literate/RetroForth.md

cp ngaImage interfaces/ri
cd interfaces/ri
../../bin/embedimage >image.c
rm ngaImage
cd ../..

cp ngaImage interfaces
cd interfaces
../bin/extend rre_windows.forth
../bin/embedimage >image_windows.c
rm ngaImage
cd ..

cp ngaImage interfaces
cd interfaces
../bin/extend rre.forth
../bin/embedimage >image.c
rm ngaImage
cc -O3 -c rre.c -o rre.o
cc -O3 -c repl.c -o repl.o
cc rre.o -lm -o rre
cc repl.o -o repl
mv rre ../bin
mv repl ../bin
rm *.o
cd ..

echo "Update Glossary"
cat words.tsv | sort >/tmp/words
mv /tmp/words words.tsv
./bin/rre glossary.forth export glossary >doc/Glossary.txt
