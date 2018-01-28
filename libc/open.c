#include <unistd.h>

long open(const char *pathname, int flags)
{
    int syscall_no = 7;
    size_t res;
    __asm__ volatile(
        "int $0x80;"
        :"=a"(res)
        :"0"(syscall_no),"b"((uint64_t)(pathname)),"c"((uint64_t)(flags))
        :"cc","memory"
    );
    return res;
}


/*int main() {
    int fd, bytes;
    char buf[10];
    fd = open("/root/a.sh", 0);
    bytes = read(fd, buf, 5);
    printf("\nRR: buf :%s", buf);
}*/

