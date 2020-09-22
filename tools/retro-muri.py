#!/usr/bin/env python3

# retro-muri is an assembler for Nga, the virtual machine at the heart of
# Retro. It is used to build the image file containing the actual Retro
# language.
#
# This will extract the code blocks in the specified file and generate an
# image file named `ngaImage`.
#
# Copyright (c) 2020, Charles Childers
#
# Usage:
#
#    retro.muri.py filename

import sys, struct

# labels stores the label names as a dictionary, with the key being
# the label name and the value being the location in memory.
#
# image stores the assembled opcodes and data.

labels = dict()
image = []

# assemble() takes a string representation of an opcode bundle,
# finds the individual opcodes, packs them into a cell-sized value,
# and returns this value.
#
# Each instruction bundle has four two character instruction names,
# with `..` used to represent a non-operation instruction.


def assemble(inst):
    insts = [
        "..",
        "li",
        "du",
        "dr",
        "sw",
        "pu",
        "po",
        "ju",
        "ca",
        "cc",
        "re",
        "eq",
        "ne",
        "lt",
        "gt",
        "fe",
        "st",
        "ad",
        "su",
        "mu",
        "di",
        "an",
        "or",
        "xo",
        "sh",
        "zr",
        "ha",
        "ie",
        "iq",
        "ii",
    ]
    a = insts.index(inst[0:2])
    b = insts.index(inst[2:4])
    c = insts.index(inst[4:6])
    d = insts.index(inst[6:8])
    o = int.from_bytes([a, b, c, d], byteorder="little", signed=False)
    return o


# muri performs two passes. The first identifies the labels
# and populates the `labels` dictionary


def pass1():
    global labels
    i = 0
    f = sys.argv[1]
    in_block = False
    with open(f, "r") as source:
        for line in source.readlines():
            if line.rstrip() == "~~~":
                in_block = not in_block
            elif in_block:
                if line[0] == "i":
                    i += 1
                if line[0] == "d":
                    i += 1
                if line[0] == "r":
                    i += 1
                if line[0] == "s":
                    i += len(line[2:].rstrip()) + 1
                if line[0] == ":":
                    labels[line[2:].rstrip()] = i


# The second pass actually assembles the instructions and fills
# the `image` array with the opcodes and data provided.


def pass2():
    global image
    i = 0
    f = sys.argv[1]
    in_block = False
    with open(f, "r") as source:
        for line in source.readlines():
            if line.rstrip() == "~~~":
                in_block = not in_block
            elif in_block:
                if line[0] == "i":
                    opcode = assemble(line[2:].rstrip())
                    image[i] = opcode
                    i += 1
                if line[0] == "d":
                    image[i] = int(line[2:].rstrip())
                    i += 1
                if line[0] == "r":
                    name = line[2:].rstrip()
                    image[i] = labels[name]
                    i += 1
                if line[0] == "s":
                    for c in line[2:].rstrip():
                        image[i] = ord(c)
                        i += 1
                    image[i] = 0
                    i += 1


# save() handles writing the image to a file


def save(filename):
    with open(filename, "wb") as file:
        j = 0
        while j < 1024:
            file.write(struct.pack("i", image[j]))
            j = j + 1


if __name__ == "__main__":
    image.extend([0] * 1024)
    pass1()
    pass2()
    save("ngaImage")
