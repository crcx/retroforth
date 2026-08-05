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
IMAGE_SIZE = 128 * 1024
image = [0] * IMAGE_SIZE
here = 0
last_dictionary_entry = 0
dictionary_entries = 0

INSTRUCTIONS = [
    "..", "li", "du", "dr", "sw", "pu", "po", "ju", "ca", "cc",
    "re", "eq", "ne", "lt", "gt", "fe", "st", "ad", "su", "mu",
    "di", "an", "or", "xo", "sh", "zr", "ha", "ie", "iq", "ii",
]
IP_MODIFYING_INSTRUCTIONS = {"ju", "ca", "cc", "re", "zr"}


def strip_comments(line):
    """Strip Pali-style trailing comments, except from string data."""
    if line.startswith("s"):
        return line

    hash_pos = line.find("#")
    while hash_pos != -1:
        if hash_pos == 0 or line[hash_pos - 1] in " \t":
            return line[:hash_pos].rstrip(" \t")
        hash_pos = line.find("#", hash_pos + 1)
    return line.rstrip(" \t")


def validate_instruction_bundle(line, line_number):
    """Validate an `i` directive against the documented Nga instruction rules."""
    if not line.startswith("i "):
        return

    bundle = line[2:]
    if len(bundle) != 8:
        raise ValueError(
            f"Error on line {line_number}: instruction bundle must contain "
            "exactly four two-character instructions"
        )

    instructions = [bundle[i : i + 2] for i in range(0, 8, 2)]
    for instruction in instructions:
        if instruction not in INSTRUCTIONS:
            raise ValueError(
                f"Error on line {line_number}: invalid Nga instruction "
                f"'{instruction}' in bundle '{bundle}'"
            )

    for index, instruction in enumerate(instructions[:-1]):
        if instruction in IP_MODIFYING_INSTRUCTIONS:
            if any(following != ".." for following in instructions[index + 1 :]):
                raise ValueError(
                    f"Error on line {line_number}: Nga instruction "
                    f"'{instruction}' must be followed only by NOPs (..) "
                    "in its bundle"
                )

# assemble() takes a string representation of an opcode bundle,
# finds the individual opcodes, packs them into a cell-sized value,
# and returns this value.
#
# Each instruction bundle has four two character instruction names,
# with `..` used to represent a non-operation instruction.


def assemble(inst):
    a = INSTRUCTIONS.index(inst[0:2])
    b = INSTRUCTIONS.index(inst[2:4])
    c = INSTRUCTIONS.index(inst[4:6])
    d = INSTRUCTIONS.index(inst[6:8])
    o = int.from_bytes([a, b, c, d], byteorder="little", signed=False)
    return o


# Each pass extracts Muri code blocks and applies the supplied handler.


def process_source(handler):
    f = sys.argv[1]
    in_block = False
    with open(f, "r") as source:
        for line_number, raw_line in enumerate(source.readlines(), start=1):
            if raw_line.rstrip() == "~~~":
                in_block = not in_block
            elif in_block:
                line = strip_comments(raw_line.rstrip("\n"))
                validate_instruction_bundle(line, line_number)
                if not line or line[0] == "c":
                    continue
                handler(line)


def parse_dict_line(line):
    fields = [field for field in line[2:].replace("\t", " ").split(" ") if field]
    if len(fields) != 3:
        raise ValueError("Dictionary entries require a name, label, and class handler")
    return fields


def dict_entry_size(name):
    return 9 + len(name.encode("utf-8")) + 1


def lookup(name):
    return labels.get(name, -1)


# The first pass records label locations and lays out every directive.


def pass1():
    global here
    here = 0

    def assemble_line(line):
        global here
        directive = line[0]
        if directive in "ir-d":
            here += 1
        elif directive == "s":
            here += len(line[2:].encode("utf-8")) + 1
        elif directive == "o":
            here = int(line[2:])
        elif directive == "*":
            here += int(line[2:])
        elif directive == "D":
            name, _, _ = parse_dict_line(line)
            here += dict_entry_size(name)
        elif directive == ":":
            name = line[2:]
            if lookup(name) != -1:
                raise ValueError(f"Fatal error: {name} already defined")
            labels[name] = here

    process_source(assemble_line)


# The second through fifth passes respectively emit opcodes, numeric data,
# strings and dictionary names, and references and dictionary headers.


def pass2():
    global here
    here = 0

    def assemble_line(line):
        global here
        directive = line[0]
        if directive == "i":
            image[here] = assemble(line[2:])
            here += 1
        elif directive == "o":
            here = int(line[2:])
        elif directive == "*":
            here += int(line[2:])
        elif directive == "D":
            name, _, _ = parse_dict_line(line)
            here += dict_entry_size(name)
        elif directive in "-rds":
            here += 1 if directive != "s" else len(line[2:].encode("utf-8")) + 1

    process_source(assemble_line)


def pass3():
    global here
    here = 0

    def assemble_line(line):
        global here
        directive = line[0]
        if directive in "ir-":
            here += 1
        elif directive == "o":
            here = int(line[2:])
        elif directive == "*":
            here += int(line[2:])
        elif directive == "d":
            image[here] = int(line[2:])
            here += 1
        elif directive == "D":
            name, _, _ = parse_dict_line(line)
            here += dict_entry_size(name)
        elif directive == "s":
            here += len(line[2:].encode("utf-8")) + 1

    process_source(assemble_line)


def pass4():
    global here
    here = 0

    def assemble_line(line):
        global here
        directive = line[0]
        if directive in "ir-d":
            here += 1
        elif directive == "o":
            here = int(line[2:])
        elif directive == "*":
            here += int(line[2:])
        elif directive == "s":
            for byte in line[2:].encode("utf-8"):
                image[here] = byte
                here += 1
            image[here] = 0
            here += 1
        elif directive == "D":
            start = here
            name, _, _ = parse_dict_line(line)
            name_bytes = name.encode("utf-8")
            for offset, byte in enumerate(name_bytes):
                image[start + 9 + offset] = byte
            image[start + 9 + len(name_bytes)] = 0
            here += dict_entry_size(name)

    process_source(assemble_line)


def pass5():
    global here, last_dictionary_entry, dictionary_entries
    here = 0
    last_dictionary_entry = 0
    dictionary_entries = 0

    def assemble_line(line):
        global here, last_dictionary_entry, dictionary_entries
        directive = line[0]
        if directive == "i":
            here += 1
        elif directive == "o":
            here = int(line[2:])
        elif directive == "*":
            here += int(line[2:])
        elif directive in "r-":
            name = line[2:]
            image[here] = lookup(name)
            if image[here] == -1:
                print(f"Lookup failed: '{name}'")
            here += 1
        elif directive == "d":
            here += 1
        elif directive == "s":
            here += len(line[2:].encode("utf-8")) + 1
        elif directive == "D":
            start = here
            name, label, class_name = parse_dict_line(line)
            size = dict_entry_size(name)
            xt = lookup(label)
            class_handler = lookup(class_name)
            if xt == -1:
                print(f"Lookup failed: '{label}'")
            if class_handler == -1:
                print(f"Lookup failed: '{class_name}'")
            image[start : start + 9] = [
                0 if dictionary_entries == 0 else last_dictionary_entry,
                xt, class_handler, 0, 0, 0, 0, 0, 0,
            ]
            last_dictionary_entry = start
            dictionary_entries += 1
            here += size

    process_source(assemble_line)


# save() handles writing the image to a file


def save(filename):
    with open(filename, "wb") as file:
        for cell in image[:here]:
            file.write(struct.pack("=i", cell))


if __name__ == "__main__":
    try:
        pass1()
        pass2()
        pass3()
        pass4()
        pass5()
        save("ngaImage")
    except ValueError as error:
        print(error)
        sys.exit(1)
