Add files to include to the code block below. Use a form
like:

    'filename include

You can either put the files (or links to them) into this
directory or use full path names to the files. You can
also use any Retro code directly.

~~~
'dict-words-listing.forth include
'allow-underscores-in-names.forth include
~~~

Save the image with anything loaded here added in. The
`retro` binary will be rebuilt using the extended image.

~~~
'../rre.image image:save
~~~
