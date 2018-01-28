#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

typedef struct commands {
    char bin_path[50]; //filename of binary executing command.
    char *args[10]; //first arg will be command and rest will be options and arguments.
    int args_len;
} command_struct;

#define O_RDONLY    0   /* Open read-only.  */
long open(const char *pathname, int flags);
int execve(const char *filename, char *const argv[], char *const envp[]);
pid_t wait(int *status);
extern void *memcpy(void *dest, const void *src, size_t n);
int non_interactive(char *argv, command_struct command_list[], char path[]);
int get_command(char command_line[]);
int split_command(char command_line[], command_struct command_list[]);
int execute_command(command_struct command, char path[]);
int change_dir(char *path);
int export_var(char *args, char path[]);
