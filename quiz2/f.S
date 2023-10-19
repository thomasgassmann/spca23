.global funcf

funcf:movl $0, %eax
      cmpl $0, %edi
      jle .L2
      movl $10, %eax
.L2:  addl $5, %eax
      ret
