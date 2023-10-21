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
        addq $-4, %r9
        
        movq %rdi, %rcx
.body:  movl (%rcx), %r8d
        cmpl %r8d, %edx
        jge .taken
.next:  addq $4, %rcx
        cmpq %rsi, %rcx
        je .lexit
        jmp .body
.taken: addq $4, %r9
        
        pushq %rdx
        pushq %rdi
        pushq %r9
        pushq %rsi
        
        movq %r9, %rsi
        movq %rcx, %rdi
        call swap
        
        popq %rsi
        popq %r9
        popq %rdi
        popq %rdx

        jmp .next
.lexit: addq $4, %r9
        
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
        jge .qexit
        
        pushq %rsi
        pushq %rdi
        
        call partition
        
        pushq %rax
        
        movq (%rsp), %rsi
        addq $-4, %rsi
        movq 8(%rsp), %rdi
        call quicksort
        
        movq (%rsp), %rdi
        addq $4, %rdi
        movq 16(%rsp), %rsi
        call quicksort
        
        popq %rax
        popq %rdi
        popq %rsi
.qexit: ret
#------------------------------------------------

#------------------------------------------------
