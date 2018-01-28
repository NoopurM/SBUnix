#define _GNU_SOURCE 
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
struct linux_dirent {
           long           d_ino;
           off_t          d_off;
           unsigned short d_reclen;
           char           d_name[];
       };

int getdents(unsigned int fd, struct linux_dirent *dirp,
                    unsigned int count) {
	long ret = 0,syscall =78, fd_long = (long)fd, count_long = (long)count, dir_long = (long)dirp;
		__asm__ (
                "movq %1, %%rax\n\t"
                "movq %2, %%rdi\n\t"
                "movq %3, %%rsi\n\t"
                "movq %4, %%rdx\n\t"
                "syscall"
                : "=r"(ret)
                : "0"(syscall), "r"(fd_long), "r"(dir_long), "r"(count_long)
                : "memory"
             );
        return ret;
}

/*size_t write(int fd, const void *buf, size_t count) {
    long ret = 0, syscall = 1, buf_addr = (long)buf, fd_long = (long)fd, count_long = (long)count;
    __asm__ (
                "movq %1, %%rax\n\t"
                "movq %2, %%rdi\n\t"
                "movq %3, %%rsi\n\t"
                "movq %4, %%rdx\n\t"
                "syscall"
                : "=r"(ret)
                : "0"(syscall), "r"(fd_long), "r"(buf_addr), "r"(count_long)
                : "memory"
             );
    return ret;
}*/

int main(int argc, char *argv[]) {
    //DIR *dir;
    struct linux_dirent *d;
	int fd=0,dircount=0,i=0;
	char buf[1024];

	if(argc > 1) {

    	fd = open(argv[1],O_RDONLY);
        printf("fd:%d",fd);
    }

    //file1 = readdir(dir);
	dircount = getdents(fd,buf,1024);
 // dircount = syscall(78, fd, buf, 1024);
  printf("hbuhb%d",dircount);

    for (i = 0; i < dircount;) {
                           d = (struct linux_dirent *) (buf + i);
                           printf("%s",d->d_name);
                           write(1, d->d_name, (strlen(d->d_name)+1));
                           i = i + d->d_reclen;
    }

    /*while(1) {
        if(file1 == NULL)
            break;
         
        //write(1, file1->d_name, strlen(file1->d_name));
        //write(1, "\t", 1);
        //file1 = readdir(dir);
    }
    write(1, "\n", 1);
    closedir(dir);*/
    return 0;
}
