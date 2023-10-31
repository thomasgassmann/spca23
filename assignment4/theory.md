# Assignment 4

## Assembly basics

### Array basics

```
char A[5];      a) 1 byte       b) 5 bytes         c) A + i         d) A[i], i[A]       e) char value = A[2];
char **C[8];    a) 8 bytes      b) 64 bytes        c) C + 8 * i     d) C[i], i[C]       e) char value = **A[2];
short *E[9];    a) 8 bytes      b) 72 bytes        c) E + 8 * i     d) E[i], i[E]       e) short value = *E[2];
int *G[7];      a) 8 bytes      b) 56 bytes        c) G + 8 * i     d) G[i], i[G]       e) int value = *G[2];
char *B[3];     a) 8 bytes      b) 24 bytes        c) B + 8 * i     d) B[i], i[B]       e) char value = *B[2];
short D[4];     a) 2 bytes      b) 8 bytes         c) B + 2 * i     d) B[i], i[B]       e) short value = D[2];
int F[4];       a) 4 bytes      b) 16 bytes        c) F + 4 * i     d) F[i], i[F]       e) int value = F[2];
```

### Addressing modes

| Operand         | Type      | Memory address | Value |
| --------------- | --------- | -------------- | ----- |
| %rax            | register  | -              | 0x2   |
| 0x210           | memory    | 0x210          | -     |
| $0x210          | immediate | -              | 0x210 |
| (%rcx)          | memory    | 0x204          | -     |
| 4(%rcx)         | memory    | 0x208          | -     |
| 5(%rcx, %rdx)   | memory    | 0x20C          | -     |
| 519(%rdx, %rax) | memory    | 524            | -     |
| 0x204(,%rax, 4) | memory    | 0x20C          | -     |
| (%rcx, %rax, 2) | memory    | 0x208          | -     |

### Arithmetic operations

| Instruction                 | Destination | Computation and result  |
| --------------------------- | ----------- | ----------------------- |
| addl %eax, (%rcx)           | memory      | M(0x204) = 0x2          |
| subl %edx, 4(%rcx)          | memory      | M(0x208) = 0x3          |
| imull (%rcx, %rax, 4), %eax | register    | %eax = 0x20C            |
| incl 8(%rcx)                | memory      | M(0x20C) = M(0x20C) + 1 |
| decl %eax                   | register    | %eax = %eax - 1         |
| subl %edx, %ecx             | register    | %ecx = 0x201            |

### leal and movl

for movl: %ecx = M(%rax + 4 * %rdx + 8)

for leal: %ecx = %rax + 4 * %rdx + 8

### Condition codes

```
(-1, -1)        (1111, 1111)    1110        OF=0,SF=1,ZF=0,CF=1
(+4, TMin)      (0100, 1000)    1100        OF=0,SF=1,ZF=0,CF=0
(TMax, TMax)    (0111, 0111)    1110        OF=1,SF=1,ZF=0,CF=0
(Tmax, -TMax)   (0111, 1001)    0000        OF=0,SF=0,ZF=1,CF=1  
(Tmin, Tmax)    (1000, 0111)    1111        OF=0,SF=1,ZF=0,CF=0
(Tmin, Tmin)    (1000, 1000)    0000        OF=1,SF=0,ZF=1,CF=1
(-1, Tmax)      (1111, 0111)    0110        OF=0,SF=0,ZF=0,CF=1
(2,3)           (0010, 0011)    0101        OF=0,SF=0,ZF=0,CF=0
```

### Reading Condition Codes with C

ce

## Assembly control flow

### Assembly Code Fragments

a)

```asm
subl %rsi, %rdi // %rsi: b, %rdi: a
movl %rdi, %rax
```

b)

```asm
leal (%edi, %edi, 4), %rax
```

c)

```asm
cmp $0, %edi
```

### Conditional branches

i)

```c
if (edx - eax <= 0) {
    eax -= edx
} else {
    edx -= eax
    eax = edx
}
```

3

i)

```c
if (ecx <= 0) {

} else {
    edx = 0;
    do {
        edx += 1;
        eax += eax
    } while (edx != ecx)
}
```

if N > 0: 2^N
if N <= 0: 1

### For loop

```c
// x: %edi, y: %esi
// x - y >= 0 => x >= y
int dog(int x, int y) {
    int i, result;
    result = 1;
    for (i = x;  x < y; i += 2) {
        result = i * result;
    }

    return result;
}
```

### Switch statement

Fragment 1: does not handle default case
Fragment 2: no, skips if a - 1 > 4
Fragment 3: no, subtract being executed multiple times
