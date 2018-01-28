#include <dirent.h>

DIR *opendir(const char *pathname) {
	int syscall_no = 9;
    size_t res;
    __asm__ volatile(
        "int $0x80;"
        :"=a"(res)
        :"0"(syscall_no),"b"((uint64_t)(pathname))
        :"cc","memory"
    );
    return (DIR *)res;
}
