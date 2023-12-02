# Solution

## Part 1: Pen and Paper

### a

b), because the exponent is all ones and the mantissa is non-zero

c), the bias is 1

c)

### b

#### 1

In binary:

1100 0000 0010 0000 0000 0000 0000 0000

sign = 1
exp = 100 0000 0
mantissa = 010 0000 0000 0000 0000 0000

bias = 127

-1.010_2 * 10^1 = -10.10 = -2.5

#### 2

-35.75

sign bit 1

in binary:

100011.11

exp should be 5, with bias that's 127 + 5 = 132 = 1000 0100

normalized:

1.0001111

therefore

1 1000 0100 0001111

1100 0010 0000 1111 0000 0000 0000 0000

in hex:

0xC20F0000

### c

the bias is 2^1 - 1 = 1

largest value: 010111 = 1.111 * 2^1 = 11.11 = 3.75

smallest value: 000001 = 0.001 * 2^0 = 0.001 = 1/8

| Bits     | e   | E   | f   | M    | V    |
| -------- | --- | --- | --- | ---- | ---- |
| 0 00 000 | 0   | 0   | 0   | 0    | 0    |
| 0 00 110 | 0   | 0   | 6/8 | 6/8  | 6/8  |
| 0 01 110 | 1   | 0   | 6/8 | 14/8 | 14/8 |
| 0 10 000 | 2   | 1   | 0   | 1    | 2    |
| 0 10 001 | 2   | 1   | 1/8 | 9/8  | 18/8 |
| 0 10 111 | 2   | 1   | 7/8 | 15/8 | 30/8 |
