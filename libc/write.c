#include <unistd.h>

size_t write(int fd, const void *buf, size_t size)
{
    volatile int syscall_no = 1;
    size_t res;
    __asm__ volatile(
        "int $0x80;"
        :"=a"(res)
        :"0"(syscall_no),"b"((uint64_t)(fd)),"c"((uint64_t)(buf)),"d"((uint64_t)(size))
        :"cc","memory"
    );
    volatile int a = 10;
    a=a;
    return res;
}

/*
int main() {
    int ret;
    ret = write(1, "aaaa", 5);
}*/
