.section .text
.global simple_arithmetic
simple_arithmetic:
# int simple_arithmetic(int a, int b)
# {
#   return a + (3 * b) + 2;
# }

leal 2(%rsi,%rsi,2), %eax
addl %edi, %eax
ret
