.section .text
.global calling_functions
calling_functions:
# int calling_functions(int a, int b)
# {
# 	return call_me(b, a);
# }

movl %esi, %edi
movq $0, %rsi
jmp call_me
