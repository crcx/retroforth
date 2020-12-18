#!/bin/sh

cd ..
retro tools/amalgamate-python.retro >benchmarks/retro.py
cd benchmarks
cp ../ngaImage .

echo Python3
time python3 retro.py times.retro
time python3 retro.py push-drop.retro

echo PyPy
time pypy retro.py times.retro
time pypy retro.py push-drop.retro

rm -f retro.py ngaImage
