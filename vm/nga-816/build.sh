#!/bin/sh
NAME=barebones

64tass -Wall -Wmacro-prefix -Wshadow --verbose-list -o ${NAME}.hex -L ${NAME}.lst --intel-hex --m65816 ${NAME}.asm
