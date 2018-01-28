.text
.global switch_to

switch_to:
    pushq %rdi
    movq %rsp, %rdi
    movq %rsi,%rsp
    popq %rdi
