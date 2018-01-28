#include <unistd.h>

void exit() {
    uint64_t syscall_no = 14;
    uint64_t a = 0;
    __asm volatile("int $0x80" : "=a" (a) : "0" (syscall_no));
}
