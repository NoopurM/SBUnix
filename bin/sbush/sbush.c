#include "sbush.h"
#include <dirent.h>
//char path[4096] = {0};
void *memset(void *s, uint32_t c, uint32_t n)
{
    uint64_t *p = (uint64_t *)s;
    while(n--)
        *p++ = c;
    return s;
}
int non_interactive(char *argv, command_struct commands[], char path[]) {
    int pos = 0, ret = 0, fd, bytes_read = 0;
    char command[1024], buf[2];
    fd = open(argv, O_RDONLY);
    if(fd) {
        while(1) {  //whole file
            bytes_read = read(fd, buf, 1);
            if (bytes_read == 0) {
                break;
            }
            pos = 0;
            while(buf[0] != '\n') { //one command

                if(buf[0] == '#') { //handle comments
                    while(buf[0] != '\n') {
                        bytes_read = read(fd, buf, 1);
                     }

                    bytes_read = read(fd, buf, 1); //get char after \n
                }

                if(buf[0] == '\n') { //handle multiple new lines/blank lines
                    while(buf[0] == '\n') {
                        bytes_read = read(fd, buf, 1);
                    }
                }

               (command)[pos++] = buf[0];
               bytes_read = read(fd, buf, 1);
            }
            (command)[pos]='\0';
            //printf("\nRR: cmd read from file :*%s*", command);
            ret = split_command(command, commands);
            ret = execute_command(commands[0], path);
        }
    }
    close(fd);
    return ret;
}

int get_command(char command[]) {
    //int pos = 0, ret = 0, bytes_read = 0;
    int ret = 0;
    //char buf[30];
    /*do {
        bytes_read = read(STDIN_FILENO, buf, 1);
        if (bytes_read == 0) {
            break;
        }
        command[pos++] = buf[0];
	} while(buf[0] != '\n');*/
    read(0, command, 30);
    //strcpy((char *)buf, (char *)command);
	//command[--pos]='\0';
    //printf("\nRR: getcmd :%s", command);
	return ret;
}

int split_command(char command_line[], command_struct commands[]) {
	int pos=0,i=0, j=0, k = 0, ret = 0;
    char tmp[6][50];
    commands[i].args_len = 0;
    do {
        if (command_line[pos] == ' ') {
            tmp[j][k] = '\0';
            commands[i].args[j] = tmp[j];
            //printf("\nRR:final arg :%s", commands[i].args[j]);
            k = 0;
            j++; pos++;
        } else if (command_line[pos] == '|') {
            tmp[j][k] = '\0';
            commands[i].args[j] = tmp[j];
            //printf("\nRR:pipe arg :%s", ((commands)[i]).args[j]);
            j = 0;
            k = 0;
            i++; pos++;
        } else {
            tmp[j][k++] = command_line[pos++];
            //printf("\nGetting arg :%s", tmp[j]);
        }
    } while(command_line[pos] != '\0');
    tmp[j][k] = '\0';
    commands[i].args[j] = tmp[j];
    //printf("\nRR:last token :%s", commands[i].args[j]);
    
    //Put last arg as NULL to get execv wokring.
    j++;
    commands[i].args[j] = NULL;
    commands[i].args_len = j-1;
    return ret;
}

int check_bin_exists(char *bin_path) {
    int fd = 0;
    fd = open(bin_path, O_RDONLY);
    if (fd  > 0) {
        //printf("\nbin path [%s] exists with exec permissions", bin_path);
        close(fd);
        return 0;
    }
    close(fd);
    return 1;
}

int export_var(char *args, char path[]) {
    int ret = 0, i=0, pos=0, found = 0;
    char name[100], value[100], *tmp = NULL, existing_var[4096];
    while (args[pos] != '=') {
        name[i++] = args[pos++];
    }
    name[i] = '\0';
    pos++;

    i=0;
    while (args[pos] != '\0') {
        if (args[pos] == ':') {
            value[i] = '\0';
            tmp = path;
            strcat(existing_var, tmp);
            found = 1;
            value[0] = '\0';
            i = 0;
            pos++;
            continue;
        }
        value[i++] = args[pos++];
    }
    value[i] = '\0';

    char *prompt = "sbush";
    if (strcmp(name, "PS1\0") == 0) {
        //printf("\nprompt [%s] changing to :%s", prompt, value);
        prompt = value;
        return ret;
    }
    prompt = prompt;
    if (found) { 
        strcat(path, ":");
        strcat(path, value);
    } else {
        //ret = setenv(name, value, 0);
    }
    //printf("\nputting env [%s] ret :%d", args, ret);
    return ret;
}

int execute_command(command_struct command, char path[]) {
    //Check if command is built-in or not
    char *all_bin_path = NULL, bin_path[1024], *command_name, tmp[1024], *env_vars = NULL;
    int pos = 0, status =0, i = 0, ret = 0, found = 0, command_args_len = 0, bytes_written = 0;
    command_args_len = command.args_len;
    command_name = command.args[0];

    /*
     * We need to handle bash specific commands seprately
     * since execv won't execute them for us.
     */
    if (strcmp(command_name, "echo") == 0) {
        while (command.args[1][pos] != '\0') {
            if (command.args[1][pos] != '$') { 
                tmp[i++] = command.args[1][pos];
            } 
            pos++;
        }
        tmp[i] = '\0';
        //printf("\nRR: getting env for :%s", tmp);
        if (strcmp(tmp, "PATH") == 0) {
            env_vars = path;
            bytes_written = write(STDOUT_FILENO, env_vars, strlen(env_vars));
            if (bytes_written == 0) {
                return ret;
            }
        } else {
            bytes_written = write(STDOUT_FILENO, command.args[1], strlen(command.args[1]));
            if (bytes_written == 0) {
                return ret;
            }
       }
        return ret;
    }

    all_bin_path = path;
    //printf("\nall bin_path :%s", all_bin_path);
    while (all_bin_path[pos] != '\0') {
        if (all_bin_path[pos] == ':') {
            bin_path[i] = '\0';
            strcat(bin_path, "/");
            strcat(bin_path, command_name);
            //printf("\nRR: checking path :%s", bin_path);
            //write(1, bin_path, strlen(bin_path)+1);
            ret = check_bin_exists(bin_path);
            if (ret == 0) {
                //printf("\nBinary found at path :%s", bin_path);
                found = 1;
                break;
            }
            i = 0;
        } else {
            bin_path[i++] = all_bin_path[pos];
        }
        pos++;
    }

    //printf("\nExecuting [%s] with args [%s] [%s] %d", bin_path, command.args[0], command.args[1], found);
    //write(1, command.args[0], strlen(command.args[0])+1);
    if (found == 1) {
        //printf("\nExecuting [%s] with args [%s] [%s]", bin_path, command.args[0], command.args[1]);
        if (fork() == 0) {
            execve(bin_path, (char * const *)command.args, NULL);
            exit(0);
        } else {
            // Wait for child process only if its not running in background.
            if (strcmp(command.args[command_args_len], "&") != 0) { 
                wait(&status);
            }
        }
    } else {
        //printf("\nBuilt in command found :%s path :%s", command.args[0], command.args[1]);
        if (strcmp(command.args[0], "cd\0") == 0) {
            ret = change_dir(command.args[1]);
        } else if (strcmp(command.args[0], "export\0") == 0) {
            ret = export_var(command.args[1], path);
        } else if (strcmp(command.args[0], "exit\0") == 0) {
            kill(0, SIGKILL);
            exit(0);
        } else {
            printf("\nBuilt command [%s] not supported yet.\n", command.args[0]);
        }
    }
    return ret;
}

int change_dir(char *path) {
    //TODO: ADD path validation
    if (path == NULL) {
        path = ".";
    }
    if ( chdir(path) != 0 )
        puts("\nCould not change directory\n");
    return 0;
}

int execute_command_1(const command_struct command) {
    //char args[50]="bin/";
    char bin_path[50];
    //char *args1[10]={0};
    char comm_args[20];
    char *usr_args[10]={0};
    for(int i=0; i <= command.args_len; i++) {
        //strcpy(command.args[i], usr_args[i]);
        memset(comm_args, 0, 20);
        strcpy(command.args[i], comm_args);
        usr_args[i] = comm_args;
    }

    //strcpy(command.args[1], args1[1]);
    if (strcmp(command.args[0], "ls\0") == 0) {
        //strcpy("ls", args1[0]);
        strcpy("bin/ls", bin_path);
    } else if (strcmp(command.args[0], "cat\0") == 0) {
        //strcpy("cat", args1[0]);
        strcpy("bin/cat", bin_path);
    } else {
        write(1, "Currently this command is not supported.", 41);
    }
    int pid = fork();
    if (pid == 0) {
        write(1, "\nin child", 9);
        //strcat(args, command.args[0]);
        execve(bin_path, usr_args, NULL);
        exit(0);
        //volatile int a = 1;
        //a=a;
    } else {
        write(1, "\nin parent", 10);
        yield();
    }
    return 1;
}

int main(int argc, char *argv[], char *envp[]) {
    //command_struct command_list[10];
    //char path[50] = {0};
    int bytes_written = 0;
    int ret = 0;
    //strcat(path, "/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin");
    //setpgid(0,0); //Need to set same id for all the processes.
    //write(1, "\nHello", 5);
    //ret = ret;
        //ret = split_command(command_line, command_list); // -- working
    //ret = ret;
    //write(1, (char *)command_line, strlen(command_line));
    //split_command(command_line, command_list);
	/*bytes_written = write(STDOUT_FILENO, "\n", 1);
    bytes_written = bytes_written;
    bytes_written = write(STDOUT_FILENO, prompt, strlen(prompt));
    bytes_written = write(STDOUT_FILENO, ">", 1);
    ret = get_command(command_line); //-- working
    ret = ret;
    bytes_written = bytes_written;*/

    /*if(argc > 1) {
        non_interactive(argv[1], command_list, path);
    }
    else {  
        //do {
            bytes_written = write(STDOUT_FILENO, "\n", 1);
            bytes_written = write(STDOUT_FILENO, prompt, strlen(prompt));
            bytes_written = write(STDOUT_FILENO, ">", 1);
            if (bytes_written == 0) {
                //break;
            }
            command_line[0] = '\0';
            ret = get_command(command_line);
            ret = split_command(command_line, command_list);
            ret = execute_command_1(command_list[0]);
            if (ret == -1) { //dead code to avoid set but not used error.
                //break;
            }
        //} while(strcmp(command_line,"exit\0") != 0);
    }*/

    //kill(0, SIGKILL);
    
    //write(1, "\nHello", 6);
    char *argv_arr1[] = {"ls", "bin"};
    //write(1, "After execv", 11);
    char *str = "\nExecuting command: ls bin/\n";
    write(1, str, strlen(str));
    char *str1 = "----------------------------\n";
    write(1,str1,strlen(str1));
    int pid = fork();
    if (pid == 0) {
        //write(1, "\nin child", 9);
        execve("bin/ls", argv_arr1, NULL);
        exit(0);
    } else {
        //write(1, "\nin parent", 10);
        yield();
    }
    
    char *str2 = "\nExecuting command: cat usr/test.c\n";
    write(1, str2, strlen(str2));
    //char *str3 = "----------------------------\n";
    //write(1,str3,strlen(str3));
    char *argv_arr[] = {"cat", "usr/test.c"};
    //write(1, "After execv", 11);
    pid = fork();
    if (pid == 0) {
        //write(1, "\nin child", 9);
        execve("bin/cat", argv_arr, NULL);
        exit(0);
    } else {
        //write(1, "\nin parent", 10);
        yield();
    }
    //write(1, "\nback", 5);
   
    // Read from console is working.
    char *str4 = "\n\nTaking inputs from user :";
    write(1, str4, strlen(str4));
    //char *str5 = "----------------------------\n";
    //write(1,str5,strlen(str5));
    //write(1,str3,strlen(str3));
    char command_line[50];
    char *prompt = "sbush";
    bytes_written = write(STDOUT_FILENO, "\n", 1);
    bytes_written = bytes_written;
    bytes_written = write(STDOUT_FILENO, prompt, strlen(prompt));
    bytes_written = write(STDOUT_FILENO, ">", 1);
    ret = get_command(command_line); //-- working
    ret = ret;
    bytes_written = bytes_written;
 
    return 0;
}
