# Naming Conventions

I use the following general conventions when naming words in RETRO.

# All

- Word names must **not** start with a prefix character
- Keep names short, but descriptive
- Spelling out is preferred over symbols

# Variables

These use TitleCase.

    Base
    Compiler
    Dictionary

# Constants

These are UPPERCASE, with a dash separating compound names.

    MAX-STRINGS
    NORTH

# Words

Most named items are words. As such, most of the conventions are under
this category.

Word names are lowercase, with a dash between compound names.

    drop
    drop-pair

Use a namespace prefix to group related words. This is a short string,
separated from the rest of the name by a colon. See Namespaces.md for a
list of the major namespaces in RETRO.

    d:for-each
    s:to-upper
    s:length
    c:vowel?
    n:negate

Words returning a flag should end in ?

    n:even?
    n:positive?
    c:vowel?

The use of a leading dash implies *not*:

    if
    -if
    c:vowel?
    c:-vowel?

When introducing words, consider including a *not* form if it makes sense.
