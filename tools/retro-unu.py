#!/usr/bin/env python3

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
