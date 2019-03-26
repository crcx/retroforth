# Forth to RETRO Cross Reference

This is a quick overview of some differences between RETRO
and traditional Forth.

| Category               | Forth                   | RETRO                         |
| ---------------------- | ----------------------- | ----------------------------- |
| Definitions            | `: name ;`              | `:name ;`                     |
| Numbers                | `100 -12`               | `#100 #-12`                   |
| Characters (interpret) | `CHAR A CHAR D`         | `$A $D`                       |
| Characters (compile)   | `[CHAR] A [CHAR] D`     | `$A $D`                       |
| Comments               | `( This is a comment )` | `(This_is_a_comment)`         |
| Pointers (interpret)   | `' Compiler`            | `&Compiler`                   |
| Pointers (compile)     | `['] Compiler           | `&Compiler`                   |
| Conditionals #1        | `IF 1 THEN`             | `[ #1 ] if`                   |
| Conditionals #2        | `NOT IF 1 THEN`         | `[ #1 ] -if`                  |
| Conditionals #3        | `IF 1 ELSE 2 THEN`      | `[ #1 ] [ #2 ] choose`        |
| Counted Loops          | `10 0 DO LOOP`          | `#10 [ ] times`               |
| Counted Loops w/Index  | `10 0 DO I LOOP`        | `#10 [ I ] times<with-index>` |
| Unconditional Loops    | `BEGIN AGAIN`           | `repeat again`                |
| Return Stack           | `10 >R ... R>`          | `#10 push ... pop`            |

RETRO conditionals and loops can be used outside of definitions, ANS ones can not.

Some forms are replaced by combinators.

    FORTH
    >R ... R>       [ ... ] dip
    DUP >R ... R>   [ ... ] sip
