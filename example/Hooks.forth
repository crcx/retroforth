## Hooks

RETRO 11 allowed nearly all definitions to be temporarily replaced by leaving space at the start for compiling in a jump. In the current RETRO I do not do this, though the technique is still useful.

The next few words add a means of doing this in RETRO 12.

To allow a word to be overridden, add a call to `hook` as the first word in the definition. This will compile a jump to the actual definition start. 

~~~
:hook (-)  #1793 , here n:inc , ; immediate
~~~

`set-hook` takes a pointer to the new word or quote and a pointer to the hook to replace. It alters the jump target.

~~~
:set-hook (aa-) n:inc store ;
~~~

The final word, `unhook`, resets the jump target to the original one.

~~~
:unhook (a-) n:inc dup n:inc swap store ;
~~~

## Test

```
:test hook  #1 #2 + n:put ;
test nl
[ 'hello s:put ] &test set-hook
test nl
&test unhook
test nl
```

