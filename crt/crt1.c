#include <stdlib.h>

void _start(void) {
  // call main() and exit() here
  __asm__ (
            
            "movq (%rsp), %rdi\n\t" //move "value" stored at address in rsp to rdi
            "movq %rsp, %rsi\n\t"  //move rsp "address" to rsi and then add 8 to get argv
            "movq $8, %rax\n\t" //since argc is int so 8 bytes
            "addq %rax, %rsi\n\t"
            "call main"
            );
  
    exit(0);
    __asm__ volatile("hlt");
}
