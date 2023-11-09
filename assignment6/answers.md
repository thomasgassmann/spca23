# Assignment 6: Linking

## Find the bugs!

a) We pass `age` instead of `&age` to `scanf`.

b) We don't free `x`. Free it.

c) We cannot assume that the values of `y` are zero-ed out.

## Symbol table

| Symbol | swap.o .symtab entry? | Symbol type | Module where defined | Section |
| ------ | --------------------- | ----------- | -------------------- | ------- |
| buf    | yes                   | extern      | main                 | .data   |
| bufp0  | yes                   | global      | swap                 | .data   |
| bufp1  | yes                   | global      | swap                 | .bss    |
| swap   | yes                   | global      | swap                 | .text   |
| temp   | no                    | -           | -                    | -       |

## Symbol references

```text
a)

REF(x.1) => DEF(x.1)
REF(x.2) => DEF(x.1)
REF(y.1) => DEF(y.2)
REF(y.1) => DEF(y.2)

b)

REF(x.1) -> DEF(x.1)
REF(x.2) -> DEF(x.1)
REF(y.1) -> DEF(y.2)
REF(y.2) -> DEF(y.2)

c)

REF(x.1) -> UNKNOWN
REF(x.2) -> UNKNOWN
REF(y.1) -> DEF(y.2)
REF(y.2) -> DEF(y.2)

d)

REF(z.1) -> DEF(z.2)
REF(z.2) -> DEF(z.2)
REF(x.1) -> ERROR
REF(x.2) -> ERROR
REF(y.1) -> DEF(y.1)
REF(y.2) -> DEF(y.1)
```

## Relocating Absolute References

| Line number | Address | Value                   |
| ----------- | ------- | ----------------------- |
| 3           | 400512  | 3c 10 60 00 00 00 00 00 |
| 5           | 40051c  | 50 10 60 00 00 00 00 00 |
| 7           | 400526  | 40 10 60 00 00 00 00 00 |
| 10          | 400533  | 38 10 60 00 00 00 00 00 |

## Relocating PC-Relative References

a) 0x400407, because %rip + f5 = 40040b + f5 = 400500, e8 being the op-code

b) f5

c) It would still be the same, because it's relative.
