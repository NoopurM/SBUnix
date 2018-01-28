#include <unistd.h>

void ps() {
    uint64_t syscall_no = 15;
    uint64_t a = 0;
    __asm volatile("int $0x80" : "=a" (a) : "0" (syscall_no));
}
