#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/defs.h>

#define X_OK    1
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define O_RDONLY    0   /* Open read-only.  */
#define O_WRONLY    1   /* Open write-only.  */
#define O_RDWR      2   /* Open read/write.  */

extern long open(const char *pathname, int flags);
int close(int fd);
size_t read(int fd, void *buf, size_t count);
size_t write(int fd, const void *buf, size_t count);
int unlink(const char *pathname);
int listf(const char *pathname);

int chdir(const char *path);
char *getcwd(char *buf, size_t size);

pid_t fork();
int execvpe(const char *file, char *const argv[], char *const envp[]);
pid_t wait(int *status);
int waitpid(int pid, int *status);
void yield();

unsigned int sleep(unsigned int seconds);

pid_t getpid(void);
pid_t getppid(void);

// OPTIONAL: implement for ``on-disk r/w file system (+10 pts)''
off_t lseek(int fd, off_t offset, int whence);
//int mkdir(const char *pathname, mode_t mode);

// OPTIONAL: implement for ``signals and pipes (+10 pts)''
int pipe(int pipefd[2]);

int access(const char *pathname, int mode);
int execv(const char *path, char *const argv[]);
int setpgid(pid_t pid, pid_t pgid);
#endif
