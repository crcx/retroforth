# Nga in Python

Nga in Python has a number of important differences from the standard
reference implementation in C.

## Cell Size

Typically cells are either 32-bit or 64-bit signed integers. Python
supports infinite precision, so this implementation does not restrict
the cell size.

It should be noted that Retro provides two words (`n:MIN` and `n:MAX`)
for determining the limits. This is used by part of the floating point
device, and may be checked by user code. The Python implementation
reports these as if the cell size is 128 bits. In the standard system,
this will only affect the use of floating point values when encoded to
normal cells.

The use of unlimited precision will be an issue if you try to save
images to run on other VM instances. At this point I'm not implementing
saving of images under the Python implementation, so this isn't a
problem currently, but if you add this, make sure to truncate the
cells to 32 or 64 bits.

## Performance

The Python implementation is slower than the C one. To help compensate
for this, it makes use of a number of techniques.

The VM detects when certain words are being called and will instead
execute native Python functions.

Of note, this currently traps and replaces:

- s:length
- s:eq?
- s:to-number
- d:lookup

String operations are slow, so providing faster length, equality,
and conversion to numbers provides a very noticable performance
improvement.

Dictionary lookups are also slow. In Retro, each token has two
lookups associated with it. First, Retro checks to see if the first
character is a `prefix:` word. If this is not the case, then the
dictionary is searched for the entire token.

The Python implementation now caches the entire dictionary as a
native dict(), and searches this instead. As with string operations,
this provides a substantial improvement.

To keep up to date, the VM also notices when `d:add-header` is
called and adds an entry to the dict before running the Retro word to
actually create the header.

It will also add entries to the dict if the overridden `d:lookup`
fails to find an entry in the cached set.

If you want to override an additional word:

- in `cached_words()`, add an entry to map in the word
- in `execute()` add an `elif` handler for the word
- make sure the `elif` handler ends in `self.ip = self.address.pop()'
