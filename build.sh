#!/bin/sh

mkdir -p bin

echo Remove Old Binaries
rm -f bin/retro-unu
rm -f bin/retro
rm -f bin/retro-embedimage
rm -f bin/retro-repl
rm -f bin/retro-extend
rm -f bin/retro-muri

echo Build Unu
cc -O2 tools/unu.c -o bin/retro-unu

echo Updating Sources
./bin/retro-unu literate/Unu.md >tools/unu.c
./bin/retro-unu literate/Muri.md >tools/muri.c

echo Build Toolchain
cc -O2 tools/embedimage.c -o bin/retro-embedimage
cc -O2 tools/extend.c -o bin/retro-extend
cc -O2 tools/muri.c -o bin/retro-muri

echo Assemble ngaImage
./bin/retro-muri literate/Rx.md
./bin/retro-extend literate/RetroForth.md

echo Prepare Image for Embedding [repl, ri]
./bin/retro-embedimage > interfaces/image.c

echo Prepare Extended Image for Embedding [bin/retro-/unix]
cp ngaImage cleanImage
./bin/retro-extend interfaces/rre.forth
./bin/retro-embedimage >interfaces/rre_image_unix.c
mv cleanImage ngaImage

echo Compile repl
cd interfaces
cc -O2 repl.c -o ../bin/retro-repl
cd ..

echo Compile rre/unix
cd interfaces
cc -O2 rre.c -lm -o ../bin/retro
cd ..

echo "Update Glossary"
LC_ALL=c sort -o sorted.tsv words.tsv
mv sorted.tsv words.tsv
./bin/retro glossary.forth export glossary >doc/Glossary.txt
