.global funcg

funcg:movl $0, %eax
      leal 3(%edi,%edi,2), %edi
      jmp .C
.S:   addl %edi, %eax
.C:   cmpl %eax, %esi
      jge .S
      ret
