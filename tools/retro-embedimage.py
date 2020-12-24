#!/usr/bin/env python3

# retro-embedimage
#
# This takes an ngaImage and generates a version that provides the
# image as a Python list. Output is written to stdout.
#
# Copyright (c) 2020, Charles Childers
#
# Usage:
#
#     retro-embedimage.py [image]


import os, sys, struct
from struct import pack, unpack

if __name__ == "__main__":
    cells = int(os.path.getsize(sys.argv[1]) / 4)
    f = open("ngaImage", "rb")
    memory = list(struct.unpack(cells * "i", f.read()))
    f.close()
    count = 0
    print('Image = [')
    for cell in memory:
        print(cell, end=', ')
        count = count + 1
        if count > 10:
            print('')
            count = 0
    print(']')
