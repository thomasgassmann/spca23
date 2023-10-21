.section .text        # Define the section for the text (code) segment.
.global quicksort
# swap: function for swapping two 32-bit values
swap:
        movl    (%rdi), %eax
        movl    (%rsi), %edx
        movl    %edx, (%rdi)
        movl    %eax, (%rsi)
        ret

partition:
        # %rdi: lo
        # %rsi: hi
        # %edx: pivot, 32bits
        # %r9: i
        # %rcx: j
        # %r8d: A[j]
        movl (%rsi), %edx
        movq %rdi, %r9
        add %r9, -4
        
        movq %rdi, %rcx
.body:  movl (%rcx), %r8d
        cmpl %edx, %r8d
        jge .taken
.next:  addq %rcx, 4
        cmpq %rsi, %rcx
        je .lexit
        jmp .body
.taken: addq %r9, 4
        
        pushq %rdx
        pushq %rdi
        pushq %r9
        
        movq %r9, %rsi
        movq %rcx, %rdi
        call swap
        
        popq %r9
        popq %rdi
        popq %rdx

        jmp .next
.lexit: addq %r9, 4
        
        movq %r9, %rdi
        movq %rsi, %rsi
        call swap
        
        movq %r9, %rax
        ret

# quicksort: main sorting function
quicksort:
        # %rdi: lo
        # %rsi: hi
        cmpq %rsi, %rdi
        jge .exit
        
        pushq %rsi
        pushq %rdi
        
        call partition
        
        pushq %rax
        
        movq (%rsp), %rsi
        add %rsi, -4
        movq 8(%rsp), %rdi
        call quicksort
        
        movq (%rsp), %rdi
        add %rdi, 4
        movq 16(%rsp), %rsi
        call quicksort
        
        popq %rax
        popq %rdi
        popq %rsi
.exit:  ret
#------------------------------------------------

#------------------------------------------------
