#!/bin/sh

echo Remove Old Binaries
rm -f bin/unu
rm -f bin/rre
rm -f bin/embedimage
rm -f bin/repl
rm -f bin/extend
rm -f bin/muri

echo Build Toolchain
cc tools/embedimage.c -o bin/embedimage
cc tools/extend.c -o bin/extend
cc tools/unu.c -o bin/unu
cc tools/muri.c -o bin/muri

echo Updating Sources
./bin/unu literate/Unu.md >tools/unu.c
./bin/unu literate/Muri.md >tools/muri.c

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

echo Prepare Extended Image for Embedding [rre/windows]
cp ngaImage cleanImage
./bin/extend interfaces/rre_windows.forth
./bin/embedimage >interfaces/rre_image_windows.c
mv cleanImage ngaImage

echo Compile repl
cd interfaces
cc -O3 repl.c -o ../bin/repl
cd ..

echo Compile rre/unix
cd interfaces
cc -O3 -lm rre.c -o ../bin/rre
cd ..

echo "Update Glossary"
LC_ALL=c sort -o sorted.tsv words.tsv
mv sorted.tsv words.tsv
./bin/rre glossary.forth export glossary >doc/Glossary.txt
