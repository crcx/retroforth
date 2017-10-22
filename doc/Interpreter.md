
Stacks
Words
Prefixes
Quotes
Combinators

Interpreter Loop:

    - repeat
      - get token
        - does token start with prefix?
          - yes: send token to prefix handler
          - no:  is token a name in the dictionary?
            - yes: push the xt of the word to stack, call class handler
            - no:  call `err:notfound`

Tokens are strings, separated by whitespace. (At a minimum, spaces; most
interfaces will also treat tab, cr, and lf as whitespace as well).

Prefixes tell RETRO how to handle tokens. Default prefixes are:

    #    Convert token to a number
    $    Return first character in token
    '    Return token as a string, replacing _ with spaces
    &    Return a pointer to a named item
    :    Start a new definition
    `    Convert token into a number and inline it into the definition
    @    Fetch a value in a named variable
    !    Store a value into a named variable
    (    Token is a comment

Tokens without a valid prefix are expected to be words in the dictionary.
RETRO will search for the token; if found it pushes a pointer (the *xt*)
to the stack and calls the class handler for the word. If not found, an
error handler is called.

Prefix handlers are named words that handle specific prefixes. They get
named like:

    prefix:#

And are called when the character after the `prefix:` identifier is the
first character of a token. Being handled as words allows for adding or
replacing them as desired.

Class handlers determine how RETRO handles words. A dictionary entry
tracks a few elements:

    - a link to the previous entry           (link)
    - a pointer to the word's start          (xt)
    - a pointer to the word's class handler  (class)
    - the name of the word                   (name)

A class handler is a word that does something with an xt based on the
system state. For instance, normal words might have a handler like:

    :class:word (a-)
      compiling? [ compile:call ] [ call ] choose ;

And *immediate* words would have a handler like:

    :class:immediate (a-)  call ;

As with prefix handlers, new classes can be added and used at any time.

RETRO does not expose a parser as part of the core language, so there
are no parsing words. Use of prefixes replaces this.	

