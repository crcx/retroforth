#!/bin/sh

echo Remove Old Binaries
rm -f bin/unu
rm -f bin/rre
rm -f bin/embedimage
rm -f bin/repl
rm -f bin/extend
rm -f bin/muri

echo Build Unu
cc -O2 tools/unu.c -o bin/unu

echo Updating Sources
./bin/unu literate/Unu.md >tools/unu.c
./bin/unu literate/Muri.md >tools/muri.c

echo Build Toolchain
cc -O2 tools/embedimage.c -o bin/embedimage
cc -O2 tools/extend.c -o bin/extend
cc -O2 tools/muri.c -o bin/muri

echo Assemble ngaImage
./bin/muri literate/Rx.md
./bin/extend literate/RetroForth.md

echo Prepare Image for Embedding [repl, ri]
./bin/embedimage > interfaces/image.c

echo Prepare Extended Image for Embedding [rre/unix]
cp ngaImage cleanImage
./bin/extend interfaces/rre.forth
./bin/embedimage >interfaces/rre_image_unix.c
mv cleanImage ngaImage

echo Compile repl
cd interfaces
cc -O2 repl.c -o ../bin/repl
cd ..

echo Compile rre/unix
cd interfaces
cc -O2 rre.c -lm -o ../bin/rre
cd ..

echo "Update Glossary"
LC_ALL=c sort -o sorted.tsv words.tsv
mv sorted.tsv words.tsv
./bin/rre glossary.forth export glossary >doc/Glossary.txt
