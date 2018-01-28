#include <stdio.h>
#include <sys/defs.h>
#include <unistd.h>

uint32_t fork(){
    uint32_t res = 0;
    int syscall_no = 4;
    //write(1, "in fork", 8);
    __asm__ volatile( 
        "int $0x80;"
        :"=a"(res)
         :"0"(syscall_no)
        :"cc","memory"
    );
    volatile int a=12;
    a=a;
    return res;

}

/*int main() {
    printf("\n%d", fork());
}*/
