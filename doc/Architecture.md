# Architecture

RETRO has a multilayer design.

At the heart of the system is a virtual machine called Nga. This emulates
a 32-bit stack processor with a MISC based instruction set.

The core RETRO language is stored as a memory image for Nga. The *image
file* contains this and is loaded on startup. It holds all of the compiled
words and data and interacts with Nga.

The third layer is the user interface. RETRO doesn't specify any required
I/O other than a console log capable of receiving a single character at a
time. Each host system can implement this and any additional desired I/O
by extending Nga.

# Specifics

On iOS the user interface is setup around an editor and an output area.
The editor extracts code from fenced regions, splits it into tokens
(elements separated by whitespace) and passes each of these into RETRO
for processing. After the tokens are processed, the console is updated.

The iOS code provides additional I/O functionality. In the `file:`
namespace there are words for creating and using files. There's a
`pb:` vocabulary for interacting with the clipboard. And a `clock:`
namespace which provides access to the system time and date. These
are mapped to higher opcodes outside of the core set used by Nga. 
