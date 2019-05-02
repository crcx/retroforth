RETRO/NATIVE
---------------------------------------------------------------
This directory contains some experiments in building a RETRO
system to run directly on hardware. As a stepping stone, it
also includes some builds which require a host kernel, but no
standard C library.

Initial Objectives:

- Reuse as much of the existing VM implementation (in C) as
  possible.
- Don't require a full blown kernel & userland to run
- Work with a completely standard RETRO image
- Minimally viable system: the basic listener (REPL)
- Work on 32-bit x86 systems

Future Goals:

- Support for more processors

  - x86-64
  - ARM (32-bit)
  - ARM (64-bit)
  - RISC-V
  - MIPS M4K (PIC32)

Current Status:

- Working builds w/minimal host dependencies:

  - FreeBSD (32-bit, x86)
  - FreeBSD (64-bit, x86)
  - Linux   (32-bit, x86)
  - Native  (32-bit, x86, multiboot)
