#!/usr/bin/env python3

# retro-embedimage
#
# This takes an ngaImage and generates a version that provides the
# image as a Python list. Output is written to stdout.
#
# Copyright (c) 2020, Charles Childers
# Copyright (c) 2021, Arland Childers
#
# Usage:
#
#     retro-embedimage.py [image]


import os, sys, struct
from struct import pack, unpack


def prints(length, priv, end=", "):
    if priv != None:
        if length == 1:
            print(priv, end=end)
        else:
            print("[{},{}]".format(length, priv), end=end)


if __name__ == "__main__":
    cells = int(os.path.getsize(sys.argv[1]) / 4)
    f = open(sys.argv[1], "rb")
    memory = list(struct.unpack(cells * "i", f.read()))
    f.close()
    count = -1  # This is counts for the extra loop at the beginning
    print("InitialImage = [", end="\n  ")
    length = 1
    priv = None

    for cell in memory:
        if cell == priv:
            length += 1
        else:
            prints(length, priv)
            priv = cell
            length = 1
            count += 1
        if count >= 10:
            print(end="\n  ")
            count = 0
    prints(length, priv, end="")
    print("\n]")
