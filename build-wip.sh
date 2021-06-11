#!/bin/sh

echo cc -DFAST -DENABLE_FLOATS -DENABLE_FILES -DENABLE_UNIX -DENABLE_RNG -DENABLE_CLOCK -DENABLE_SCRIPTING -O2 -pipe -o ../../bin/retro-wip retro-wip.c -lm
cd vm/nga-c && cc -DFAST -DENABLE_FLOATS -DENABLE_FILES -DENABLE_UNIX -DENABLE_RNG -DENABLE_CLOCK -DENABLE_SCRIPTING -O2 -pipe -o ../../bin/retro-wip retro-wip.c -lm
