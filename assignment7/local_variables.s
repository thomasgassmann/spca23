.section .text
.global local_variables
local_variables:
# int local_variables()
# {
# 	int local = 3;
# 	return call_me(local, &local);
# }

pushq $3
movl $3, %edi
movq %rsp, %rsi
call call_me
popq %rax
ret
