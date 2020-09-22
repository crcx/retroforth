#!/usr/bin/env python3

import sys, struct

labels = dict()
image = []

# the first pass identifies the labels and fills in a dictionary
# that will be used in the second pass


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
