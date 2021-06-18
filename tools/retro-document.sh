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
# - retro-unu
#
# Usage:
#
# ./describe sourcefile

retro-unu $1 | tr " " "\n" | sed '/^[[:space:]]*$/d' | grep -Ev "^[\\\^\.\"\@\!\$\`:&'#$.\(\|]" | sort | uniq   | tr "\n" "\0" | xargs -0 retro-describe | cat -s
retro-unu $1 | tr " " "\n" | sed '/^[[:space:]]*$/d' | cut -c1-1 | sort | uniq   | grep "[\\\^\.\"\@\!\$\`:&'#$.\(\|]" | sed 's/^/sigil:/'  | tr "\n" "\0" | xargs -0 retro-describe | cat -s
