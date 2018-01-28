#include <unistd.h>

int catf(const char *pathname) {
    int syscall_no = 13;
    size_t res;
    __asm__ volatile(
        "int $0x80;"
        :"=a"(res)
        :"0"(syscall_no),"b"((uint64_t)(pathname))
        :"cc","memory"
    );
    return res;
}
