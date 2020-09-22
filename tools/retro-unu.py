#!/usr/bin/env python3

# retro-unu is a tool for extracting code from literate sources. It
# will write output to stdout.
#
# A code block starts with ~~~ on a line by itself and ends with a
# second ~~~.
#
# Copyright (c)2020, Charles Childers
#
# Usage:
#
#    retro-unu.py filename

import sys

if __name__ == "__main__":
    f = sys.argv[1]
    in_block = False
    with open(f, "r") as source:
        for line in source.readlines():
            if line.rstrip() == "~~~":
                in_block = not in_block
            elif in_block:
                print(line.rstrip())
