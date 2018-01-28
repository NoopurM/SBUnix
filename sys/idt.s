.text
.macro pushall
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
.endm

.macro popall
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
.endm

.global fork_isr
fork_isr:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    iretq


.global isr_handler
isr_handler:
    pushall
    call generic_handler_isr       # A special call, preserves the 'eip' register
    popall
    iretq           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

.global general_protection_handler
general_protection_handler:
    pushall
    call general_protection_handler_isr       # A special call, preserves the 'eip' register
    popall
    iretq           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

.global page_fault_handler
page_fault_handler:
    pushall
    call page_fault_handler_isr      # A special call, preserves the 'eip' register
    addq $8, %rsp
    popall
    iretq           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

.global isr32
isr32:
    pushall
    call timer_isr       # A special call, preserves the 'eip' register
    popall
    iretq           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

.global isr33
isr33:
    pushall
    call keyboard_handler       # A special call, preserves the 'eip' register
    popall
    iretq           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

.global isr128
isr128:
    cli
    pushall
    call syscall_handler       # A special call, preserves the 'eip' register
    popall
    iretq           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!
