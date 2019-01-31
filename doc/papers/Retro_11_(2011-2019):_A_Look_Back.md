# Retro 11 (2011 - 2019): A Look Back

So it's now been about five years since the last release of
Retro 11. While I still see some people obtaining and using it,
I've moved on to the twelth generation of Retro. It's time for
me to finally retire Retro 11.

As I prepare to do so, I thought I'd take a brief look back.

Retro 11 began life in 2011. It grew out of Retro 10, which
was the first version of Retro to not be written in x86 assembly
language. For R10 and R11, I wrote a portable virtual machine
(with numerous implementations) and the Forth dialect was kept
in an image file which ran on the VM.

Retro 10 worked, but was always a bit too sloppy and changed
drastically between releases. The major goal of Retro 11 was
to provide a stable base for a five year period. In retrospect,
this was mostly achieved. Code from earlier releases normally
needed only minor adjustments to run on later releases, though
newer releases added significantly to the language.

There were seven releases.

- Release 11.0: 2011, July
- Release 11.1: 2011, November
- Release 11.2: 2012, January
- Release 11.3: 2012, March
- Release 11.4: 2012, July
- Release 11.5: 2013, March
- Release 11.6: 2014, August

Development was fast until 11.4. This was the point at which
I had to slow down due to RSI problems. It was also the point
which I started experiencing some problems with the metacompiler
(as discussed previously).

Retro 11 was flexible. All colon definitions were setup as hooks,
allowing new functionality to be layered in easily. This allowed
the later releases to add things like vocabularies, search order,
tab completion, and keyboard remapping. This all came at a cost
though: later things could use the hooks to alter behavior of
existing words, so it was necessary to use a lot of caution to
ensure that the layers didn't break the earlier code.

The biggest issue was the I/O model. Retro 11 and the Ngaro VM
assumed the existence of a console environment. All input was
required to be input at the keyboard, and all output was to be
shown on screen. This caused some problems. Including code from
a file required some tricks, temporarily rewriting the keyboard
input function to read from the file. It also became a major
issue when I wrote the iOS version. The need to simulate the
keyboard and console complicated everything and I had to spend
a considerable amount of effort to deal with battery performance
resulting from the I/O polling and wait states.

But on the whole it worked well. I used Retro 11.6 until I
started work on Retro 12 in late 2016, and continued running
some tools written in R11 until the first quarter of last year.

The final image file was 23,137 cells (92,548 bytes). This was
bloated by keeping some documentation (stack comments and short
descriptions) in the image, which started in 11.4. This contained
269 words.

I used Retro 11 for a wide variety of tasks. A small selection
of things that were written includes:

- a pastebin
- front end to ii (irc client)
- small explorations of interactive fiction
- irc log viewer
- tool to create html from templates
- tool to automate creation of an SVCD from a set of photos
- tools to generate reports from data sets for my employer

In the end, I'm happy with how Retro 11 turned out. I made some
mistakes in embracing too much complexity, but despite this it
was a successful system for many years.
