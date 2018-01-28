#ifndef _DIRENT_H
#define _DIRENT_H

#define NAME_MAX 255

#include<sys/tarfs.h>

struct dirent {
 char d_name[NAME_MAX+1];
};

struct DIR {
    struct elf_node *node;
    struct dirent dentry;
    int child_no;
};
typedef struct DIR DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

#endif
