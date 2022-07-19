# Custom Extensions

This is a system for allowing you to easily add your own
extensions to RETRO on Unix systems.

The first extension uses the scripting interface to get source
data for words, and record this as part of the header.

~~~
{{
  'Sources d:create #128 allot
  :known? (s-sf) dup &Sources a:contains/string? ;
  :index  (s-s)  &Sources swap a:index/string
                 &Sources swap a:fetch ;
  :record (s-s)  s:keep dup &Sources v:inc
                 @Sources &Sources + store ;

  [ script:current-file known? [ index ] [ record ] choose
    [ &d:add-header #2 + call ] dip
    d:last d:source store d:rehash ] &d:add-header set-hook
}}
~~~


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
