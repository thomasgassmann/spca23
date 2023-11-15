.section .text
.global loops
loops:
# int loops(int a)
# {
# 	int n_iterations = 0;
# 	while(a > 0)
# 	{
# 		call_me(0,NULL);
# 		a = a - 1;
# 		n_iterations = n_iterations + 1;
# 	}
# 	return n_iterations;
# }

cmpl $0, %edi
jle noit

push %rbp
push %rdi
push %rdi

movl $0, %edi
movq $0, %rsi

loop:
call call_me
subl $1, (%rsp)
cmpl $0, (%rsp)
jne loop

pop %rdi
pop %rax
leave
ret

noit:
xorl %eax, %eax
ret