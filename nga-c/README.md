---------------------------------------------------------------
I'm exploring some things around the VM implementation and
tooling; this directory contains the relevant files for this.

Things I'm working on in this:

[x] Single file implementation
[x] Modular I/O (enable/disable at compile time)
[x] Faster opcode processing
[x] Removal of image saving from the C sources
[x] Building extended image w/o retro-extend in C
[x] Assemble new image w/o retro-muri in C
[ ] Support for embedding device words in the C source
[ ] Embed image in the C source using tool written in Retro
---------------------------------------------------------------
Specifics:

Retro's build process is a little messy. We first compile the
assembler, then retro-extend (which is an Nga-in-C, with a
reduced set of I/O). These are used to assemble the kernel,
merge in the stdlib and any device words, then the VM is
compiled, user-level additions are added to a new image, and
then the source is updated before being rebuilt.

In this, the VM is built, then is used to assemble and build a
new image, eliminating the need for separate retro-extend and
retro-muri binaries.

In the longer term, there is benefit to having a standalone
set of supporting tools, so I'm not going to eliminate them,
but using Retro itself for more is something worth doing IMO.
---------------------------------------------------------------
