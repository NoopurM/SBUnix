#include <stdio.h>
long kill(long pid, int sig) 
{
    long ret = 0, syscall = 62, pid_long = (long)pid, sig_long = (long)sig;
    __asm__ ( 
                "movq %1, %%rax\n\t" 
                "movq %2, %%rdi\n\t"
                "movq %3, %%rsi\n\t"
                "syscall"
                : "=r"(ret)
                : "0"(syscall), "r"(pid_long), "r"(sig_long)
                : "memory"
             ); 
    return ret;
}

/*int main() {
    kill(14635, 9);
}*/

