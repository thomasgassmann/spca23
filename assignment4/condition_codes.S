.section .text        # Define the section for the text (code) segment.

.global getflags           # Declare the "getflags" function as a global symbol so it can be used externally.

getflags:                  # Start of the "getflags" function.
    pushq   %rbp      # Push the value of the base pointer (rbp) onto the stack.
                      # This is done to save the current function's base pointer.

    movq    %rsp, %rbp # Set the base pointer (rbp) to the current stack pointer (rsp).
                      # This establishes a new stack frame for the function.

    # Student TODO: Implement
    #-----------------------------------
    cmpq %rdi, %rsi
    pushf
    movw (%rsp), %ax
    popf
    #-----------------------------------
    popq    %rbp           # Restore the previous base pointer by popping it from the stack.

    ret                    # Return from the function, which will pop the return address from the stack and jump to it.
