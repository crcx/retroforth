# Hyperstatic Global Environment

This now brings up an interesting subpoint. Retro provides a *hyper-
static global environment.* This can be difficult to explain, so let's
take a quick look at how it works:

~~~
    #1000 'a var<n>
    :scale (x-y) @a * ;
    #3 scale putn
    >>> 3000
    #100 !a
    #3 scale putn
    >>> 300
    #5 'a var<n>
    #3 scale putn
    >>> 300
    @a putn
    >>> 5
~~~

Output is marked with **\>\>\>**.

Note that we create two variables with the same name (*a*). The definition
for `scale` still refers to the old variable, even though we can no longer
directly manipulate it.

In a hyper-static global environment, functions continue to refer to the
variables and earlier functions that existed when they were defined. If
you create a new variable or function with the same name as an existing
one, it only affects future code.
