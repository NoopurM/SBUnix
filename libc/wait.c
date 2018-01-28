#include <unistd.h>
#include <stdio.h>
pid_t wait(int *status) {
    long ret = 0, syscall = 61, status_long = (long)status;
    __asm__ ( 
                "movq %1, %%rax\n\t" 
                "movq %2, %%rdi\n\t"
                "syscall"
                : "=r"(ret)
                : "0"(syscall), "r"(status_long)
                : "memory"
             ); 
    return ret;
}


/*int main() {
    int pid = 0;
    if (fork() == 0) {
        printf("\nin child process");
        exit(0);
    } else {
        pid = wait(NULL);
    }
    printf("%d %d", getpid(), pid);
}*/

