#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
//#include <fcntl.h>
//#include <sys/wait.h>

typedef struct commands {
    char *bin_path; //filename of binary executing command.
    char **args; //first arg will be command and rest will be options and arguments.
    int args_len;
} command_struct;

int spawn_proc (int in, int out, command_struct command);
int fork_pipes (int n, command_struct commands[]);
int non_interactive(char *argv, command_struct command_list[]);
int get_command(char **command_line);
int split_command(char *command_line, command_struct command_list[]);
int execute_command(command_struct command);
int change_dir(char *path);
int export_var(char *args);
