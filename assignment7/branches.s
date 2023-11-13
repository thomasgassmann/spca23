.section .text
.global branches
branches:
# int branches(int a)
# {
# 	if(a == 12)
# 	{
# 		return 0;
#	}
# 	else{
# 		return 1;
# 	}
# }

movl $0, %esi
movl $1, %eax
cmpl $12, %edi
cmove %esi, %eax
ret
