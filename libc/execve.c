#include <unistd.h>

uint64_t execve(char *filename, char *argv[], char *envp[]) {
    uint64_t syscall_no = 5;
    uint64_t res;
    __asm__ volatile(
        "int $0x80;"
        :"=a"(res)
        :"0"(syscall_no),"b"((uint64_t)(filename)),"c"((uint64_t)(argv)),"d"((uint64_t)(envp))
        :"cc","memory"
    );
    return 0;
}
