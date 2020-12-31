# Custom Extensions

This is a system for allowing you to easily add your own
extensions to RETRO on Unix systems.

There are two options:

## Manual Additions

Add files to include to the code block below. Use a form
like:

    'filename include

You can either put the files (or links to them) into this
directory or use full path names to the files. You can
also use any Retro code directly.

~~~
'dict-words-listing.forth include
~~~

## Automatic Extensions

This does not require manual editing of this file. To use
this:

- copy (or symlink) the extensions into the `extensions` subdirectory
- run `make update-extensions`
- run `make`

This will build RETRO, generate a new `load-extensions.retro` and
then rebuild, including the extensions.

~~~
'load-extensions.retro include
~~~

## Final Bits

Save the image with anything loaded here added in. The
`retro` binary will be rebuilt using the extended image.

~~~
'../rre.image image:save
~~~
