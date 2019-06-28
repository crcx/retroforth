#!/bin/sh
for f in *.forth; do mv -- "$f" "${f%.forth}.retro" ; done
for f in *.retro; do fossil rename -- "${f%.retro}.forth" "$f" ; done
