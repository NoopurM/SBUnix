#include <dirent.h>

struct dirent *readdir(DIR *dirp) {
	
	int syscall_no = 10;
    size_t res;
    __asm__ volatile(
        "int $0x80;"
        :"=a"(res)
        :"0"(syscall_no),"b"((uint64_t)(dirp))
        :"cc","memory"
    );
    return (struct dirent *)res;
}
