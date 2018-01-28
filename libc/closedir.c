#include <dirent.h>

int closedir(DIR *dirp) {
    int syscall_no = 11;
    size_t res;
    __asm__ volatile(
        "int $0x80;"
        :"=a"(res)
        :"0"(syscall_no),"b"((uint64_t)(dirp))
        :"cc","memory"
    );
    return res;
}
