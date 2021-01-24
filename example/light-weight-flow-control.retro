# Lightweight Flow Control

These were adapted from HerkForth.

    | 0=;  |  n- | exit word if TOS = 0                                   |
    | 0<>; |  n- | exit word if TOS <> 0                                  |
    | <;   | nn- | exit word if NOS < TOS                                 |
    | >;   | nn- | exit word if NOS > TOS                                 |
    | <>;  | nn- | exit word if NOS <> TOS                                |
    | if;  |  f- | exit word if TOS is TRUE                               |
    | ?;   |  f- | exit word if TOS is TRUE. Leave Flag on stack if TRUE. |

~~~
:0=;  n:zero? [ as{ 'popopodr i 'drdrre.. i }as ] if ;
:0<>; n:zero? [ as{ 'popopodr i 'drdrre.. i }as ] -if ;
:<;   lt?     [ as{ 'popopodr i 'drdrre.. i }as ] if ;
:>;   gt?     [ as{ 'popopodr i 'drdrre.. i }as ] if ;
:<>;  -eq?    [ as{ 'popopodr i 'drdrre.. i }as ] if ;
:if;          [ as{ 'popopodr i 'drdrre.. i }as ] if ;
:?;   dup     [ as{ 'popopodr i 'drdrre.. i }as ] if drop ;
~~~

# Tests

```
:test (n-) n:even? if; 'Odd! s:put nl ;

#1 test
#2 test
```

```
nl '----------------- s:put nl
:test (n-) n:even? ?; 'Odd! s:put nl ;
#1 test dump-stack reset nl
#2 test dump-stack reset nl
```
