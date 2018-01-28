#ifndef _TARFS_H
#define _TARFS_H
#include <sys/defs.h>

#define DIRECTORY_TYPE 5
#define FILE_TYPE 0
#define ROOTFS 999

extern char _binary_tarfs_start;
extern char _binary_tarfs_end;

struct posix_header_ustar {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char checksum[8];
  char typeflag[1];
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char pad[12];
};

struct fs_node {
    int parent_index;
    char filename[100];
    uint64_t filesize;
    int type;
    uint64_t header_address;
};


struct fd_node {
    struct elf_node *node;
    int fd;
    struct fd_node *next;
};

struct elf_node {
    int type;
    struct elf_node *parent;
    struct elf_node *child[30];
   // int no_of_children;
    char filename[100];
    uint64_t filesize;
    uint64_t header_address;
    uint64_t last_read;
};

struct elf_node *rootfs;
void start_tarfs();
void parse_tarheader(struct posix_header_ustar *tarheader);
uint64_t parse_filesize(struct posix_header_ustar *tarheader);
void parse_filename(struct posix_header_ustar *tarheader);
uint64_t atoi(char *c);
uint64_t power(uint64_t x, int e);
uint64_t OctToDec(uint64_t oct);
void create_entry(int entry_no, struct posix_header_ustar *tarheader, int type);
void parse_dir_name(int entry_no);
void parse_file_name(int entry_no);
int find_parent(char *parent, int entry_no);
void print_fs_entries(int max_entry);
void init_tree();
int strtok(char filename[50], char res[20][50]);
struct elf_node *insert_node(int filesize, char filename[50], int type, struct posix_header_ustar *tarheader);
struct elf_node *search_node(char filename[50]);
struct posix_header_ustar *get_elf_file_from_tarfs(char filename[50]);
#endif
