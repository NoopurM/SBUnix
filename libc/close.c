#include <unistd.h>

int close(int fd)
{
    int syscall_no = 8;
    size_t res;
    __asm__ volatile(
        "int $0x80;"
        :"=a"(res)
        :"0"(syscall_no),"b"((uint64_t)(fd))
        :"cc","memory"
    );
    return res;
}
