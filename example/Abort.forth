There are people who consider `abort` to be the most
fundamental  building block in writing programs. For
them, it is not just a debugging tool.

~~~
:s:shout (s-) '!_ s:prepend s:put nl 'Stack_:_ s:put dump-stack nl ;
{{
  'Depth var
  :message (-) 'abort_with_trail_invoked s:shout nl
    'Do__reset__to_clear_stack. s:put nl ;
  :put-name (a-) fetch d:lookup-xt
    dup n:-zero? [ d:name s:put nl ] [ drop ] choose ;
---reveal---
  :trail repeat pop put-name again ;
  :abort (-?) depth !Depth message trail ;
  :s:abort (s-) 's:abort_:_ s:prepend s:put nl abort ;
}}
~~~

`trail` leaves garbage on the stack.
This garbage is difficult to shake off since `trail`
destroys the contents of the return stack. The easiest
way I have found so far is to `reset` manually.

To test, run the following code interactively:

```
:a 'aaa s:abort ;
:b a ;
:c b ;
c
reset
```

## assert

~~~
:assert (q-) call [ abort ] -if ;
:assert.verbous (q-) call [ 'assert_:_fail s:abort ] -if ;
~~~

If `0;` is invoked with an empty stack, it fails and
kills the session.

```
:t 'before_ s:put 0; 'after s:put nl ;
'#0_t_nl_#1_t_nl s:evaluate
'reset_t s:evaluate (--_kills_session
```

If this is dangerous, place an assertion before `0;` .

