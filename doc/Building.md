# Building RETRO

To rebuild the core binaries, a simple `make` should suffice. This will
leave you with `bin/rre` and `bin/repl`, as well as the `bin/listener`
(wrapper for rre).

If you make changes to the image sources (in `literate`) or to
`interfaces/rre.forth`, you'll need to do a full build
(toolchain + image + interfaces). This can be done by issuing a
`make all` command.
