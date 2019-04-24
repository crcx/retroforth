#!/bin/sh

# This is a quick and dirty shell script to extract the words
# in a source file and return the descriptions of them.
#
# Requirements:
#
# In your $PATH:
#
# - retro
# - retro-describe
#
# Usage:
#
# ./describe sourcefile

# Extract    | Split       | Strip Blanks            | Sort | Filter | Terminate    | Generate                               | Strip Blanks
retro-unu $1 | tr " " "\n" | sed '/^[[:space:]]*$/d' | sort | uniq   | tr "\n" "\0" | xargs -0 -n1 retro-describe | cat -s

# Extract    | Split       | Strip Blanks            | Get first | Sort | Filter | Find Prefixes                   | Generate word names | Terminate    | Generate                               | Strip Blanks
retro-unu $1 | tr " " "\n" | sed '/^[[:space:]]*$/d' | cut -c1-1 | sort | uniq   | grep "[\.\"\@\!\$\`:&'#$.\(\|]" | sed 's/^/prefix:/'  | tr "\n" "\0" | xargs -0 -n1 retro-describe | cat -s
