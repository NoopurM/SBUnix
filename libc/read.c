#include <unistd.h>

size_t read(int fd, void *buf, size_t size)
{
    int syscall_no = 3;
    size_t res;
    __asm__ volatile(
        "int $0x80;"
        :"=a"(res)
        :"0"(syscall_no),"b"((uint64_t)(fd)),"c"((uint64_t)(buf)),"d"((uint64_t)(size))
        :"cc","memory"
    );
    return res;
}


/*
int main() {
    char buf[10];
    int ret;
    ret = read(0, buf, 5);
    printf("\n%s %d", buf, ret);
}*/
