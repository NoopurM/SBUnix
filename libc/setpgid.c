#include <stdio.h>
int setpgid(long pid, long pgid) {
    long ret = 1, syscall = 109, pid_long = (long)pid, pgid_long = (long)pgid;
    __asm__ ( 
                "movq %1, %%rax\n\t" 
                "movq %2, %%rdi\n\t"
                "movq %3, %%rsi\n\t"
                "syscall"
                : "=r"(ret)
                : "0"(syscall), "r"(pid_long), "r"(pgid_long)
                : "memory"
             ); 
    return ret;
}


/*int main() {
    printf("%d", setpgid(0, 0));
}*/

