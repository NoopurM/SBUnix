#include "sbush.h"
#include <fcntl.h>

static char *prompt = "sbush";
int non_interactive(char *argv, command_struct commands[]) {
    int pos = 0, ret = 0, fd, bytes_read = 0;
    char *command, buf[2];
    fd = open(argv, O_RDONLY);
    if(fd) {
        while(1) {  //whole file
            bytes_read = read(fd, buf, 1);
            if (bytes_read == 0) {
                break;
            }
            pos = 0;
            command = NULL;
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

               command = realloc(command, (pos+1)*sizeof(char));
               (command)[pos++] = buf[0];
               bytes_read = read(fd, buf, 1);
            }
            command = realloc(command, (pos+1)*sizeof(char));
            (command)[pos]='\0';
            //printf("\nRR: cmd read from file :*%s*", command);
            ret = split_command(command, commands);
            ret = execute_command(commands[0]);
        }
    }
    close(fd);
    return ret;
}

int spawn_proc (int in, int out, command_struct command)
{
  pid_t pid;

  if ((pid = fork ()) == 0)
    {
      if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }
	
	//	return execute_command(command);
//      return execvp (cmd->argv [0], (char * const *)cmd->argv);
    printf("**%s**",command.args[0]);
    return execve(command.args[0], command.args,NULL);
      //execve(bin_path, command_args, NULL);
    }

  return pid;
}


int fork_pipes (int n, command_struct commands[])
{ 
  int i;
  //pid_t pid; 
  int in, fd [2];
  
  /* The first process should get its input from the original file descriptor 0.  */
  in = 0;
  
  /* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
  for (i = 0; i < n - 1; ++i)
    { 
      pipe (fd);
      
      /* f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
      spawn_proc (in, fd [1], commands[i]);
      
      /* No need for the write end of the pipe, the child will write here.  */
      close (fd [1]);
      
      /* Keep the read end of the pipe, the next child will read from there.  */
      in = fd [0];
    }
  
  /* Last stage of the pipeline - set stdin be the read end of the previous pipe
     and output to the original file descriptor 1. */
  if (in != 0)
    dup2 (in, 0);
  
  /* Execute the last stage with the current process. */
  //return execvp (cmd [i].argv [0], (char * const *)cmd [i].argv);
//	return execute_command(commands[i]);
return execve(commands[i].args[0], commands[i].args,NULL);
}

int get_command(char **command) {
    int pos = 0, ret = 0, bytes_read = 0;
    char buf[1];
    do {
        bytes_read = read(STDIN_FILENO, buf, 1);
        if (bytes_read == 0) {
            break;
        }
        *command = (char *)realloc(*command, (pos+1)*sizeof(char));
        (*command)[pos++] = buf[0];
	} while(buf[0] != '\n');
	(*command)[--pos]='\0';
    //printf("\nRR: getcmd :%s", *command);
	return ret;
}

int split_command(char *command_line, command_struct commands[]) {
	int pos=0,i=0, j=0, k = 0, ret = 0;
    commands[i].args_len = 0;
    commands[i].args = (char **)malloc(sizeof(char *));
    commands[i].args[j] = NULL;
    do {
        if (command_line[pos] == ' ') {
            if(k!=0) {
            commands[i].args[j] = (char *)realloc(commands[i].args[j], (k+1)*sizeof(char));
            commands[i].args[j][k] = '\0';
            printf("\nRR:final arg :$%s$k:%d -- i:%dj:%d", commands[i].args[j],k,i,j);
            k = 0;
            j++; pos++;
            commands[i].args = realloc(commands[i].args, (j+1)*sizeof(char *));
            ((commands)[i]).args[j] = NULL;
           }
           else {
               pos++;
           }
        } else if (command_line[pos] == '|') {
            ((commands)[i]).args[j] = (char *)realloc(((commands)[i]).args[j], (k+1)*sizeof(char));
            if(k == 0)
                   (commands)[i].args[j] = NULL;
            else {
                   //((commands)[i]).args[j][k] = '\0';
                   j++;
                   printf("j - *%d*",j);
            }
            printf("\nRR:pipe arg :$%s$k:%d -- i:%dj:%d", ((commands)[i]).args[j],k,i,j);
            j = 0;
            k = 0;
            i++; pos++;
            commands[i].args = (char **)malloc(sizeof(char *));
            ((commands)[i]).args[j] = NULL;
        } else {
            commands[i].args[j] = (char *)realloc(commands[i].args[j], (k+1)*sizeof(char));
            commands[i].args[j][k] = command_line[pos];
            printf("%c",commands[i].args[j][k]);
            k++;
            pos++;
        }
    } while(command_line[pos] != '\0');
    ((commands)[i]).args[j] = (char *)realloc(((commands)[i]).args[j], (k+1)*sizeof(char));
    ((commands)[i]).args[j][k] = '\0';
    printf("\nRR:last token :%s k:%d -- i:%dj:%d", ((commands)[i]).args[j],k,i,j);
    
    //Put last arg as NULL to get execv wokring.
    j++;
    commands[i].args = realloc(commands[i].args, (j+1)*sizeof(char *));
    ((commands)[i]).args[j] = NULL;
    commands[i].args_len = j-1;
    return ret;
}

int check_bin_exists(char *bin_path) {
    if ((access(bin_path, X_OK)) == 0) {
        //printf("\nbin path [%s] exists with exec permissions", bin_path);
        return 0;
    }
    return 1;
}

int export_var(char *args) {
    int ret = 0, i=0, pos=0;
    char *name = NULL, *value = NULL, *existing_var = NULL;
    while (args[pos] != '=') {
        name = realloc(name, (i+1) * sizeof(char));
        name[i++] = args[pos++];
    }
    name = realloc(name, (i+1) * sizeof(char));
    name[i] = '\0';
    pos++;

    i=0;
    while (args[pos] != '\0') {
        if (args[pos] == ':') {
            value = realloc(value, (i+1) * sizeof(char));
            value[i] = '\0';
            existing_var = getenv(value);
            value = NULL;
            i = 0;
            pos++;
            continue;
        }
        value = realloc(value, (i+1) * sizeof(char));
        value[i++] = args[pos++];
    }
    value = realloc(value, (i+1) * sizeof(char));
    value[i] = '\0';

    if (strcmp(name, "PS1\0") == 0) {
        //printf("\nprompt [%s] changing to :%s", prompt, value);
        prompt = value;
        return ret;
    }
    if (existing_var) { 
        existing_var = realloc(existing_var, (strlen(existing_var) + strlen(value) + 2)*sizeof(char));
        strcat(existing_var, ":");
        strcat(existing_var, value);
        //printf("\nsetting value :%s", existing_var);
        ret = setenv(name, existing_var, 0);
    } else {
        //printf("\nsetting value :%s", value);
        ret = setenv(name, value, 0);
    }
    //printf("\nputting env [%s] ret :%d", args, ret);
    return ret;
}

int execute_command(command_struct command) {
    //Check if command is built-in or not
    char *all_bin_path = NULL, *bin_path = NULL, *command_name = NULL, *tmp = NULL, **command_args = NULL, *env_vars=NULL;
    int pos = 0, i = 0, ret = 0, status = 0, found = 0, command_args_len = 0, bytes_written = 0;
    command_args = command.args;
    command_args_len = command.args_len;
    command_name = command_args[0];

    /*
     * We need to handle bash specific commands seprately
     * since execv won't execute them for us.
     */
    if (strcmp(command_name, "echo") == 0) {
        while (command_args[1][pos] != '\0') {
            if (command_args[1][pos] != '$') { 
                tmp = realloc(tmp, (i+1) * sizeof(char));
                tmp[i++] = command_args[1][pos];
            } 
            pos++;
        }
        tmp = realloc(tmp, (i+1) * sizeof(char));
        tmp[i] = '\0';
        //printf("\nRR: getting env for :%s", tmp);
        if (getenv(tmp)) {
            env_vars = getenv(tmp);
            bytes_written = write(STDOUT_FILENO, env_vars, strlen(env_vars));
            if (bytes_written == 0) {
                return ret;
            }
        } else {
            bytes_written = write(STDOUT_FILENO, command_args[1], strlen(command_args[1]));
            if (bytes_written == 0) {
                return ret;
            }
       }
        return ret;
    }

    all_bin_path = getenv("PATH");
    //printf("\nall bin_path :%s", all_bin_path);
    while (all_bin_path[pos] != '\0') {
        if (all_bin_path[pos] == ':') {
            bin_path[i] = '\0';
            bin_path = realloc(bin_path, (strlen(command_name) + strlen(bin_path) + strlen("/") + 1)*sizeof(char));
            strcat(bin_path, "/");
            strcat(bin_path, command_name);
            ret = check_bin_exists(bin_path);
            if (ret == 0) {
                //printf("\nBinary found at path :%s", bin_path);
                found = 1;
                break;
            }
            i = 0;
            bin_path = NULL;
        } else {
            bin_path = realloc(bin_path, (i+1) * sizeof(char));
            bin_path[i++] = all_bin_path[pos];
        }
        pos++;
    }

    if (found == 1) {
        printf("\nExecuting [%s] with args [%s] [%s]", bin_path, command_args[0], command_args[1]);
        if (fork() == 0) {
            execve(bin_path, command_args, NULL);
        } else {
            // Wait for child process only if its not running in background.
            if (strcmp(command_args[command_args_len], "&\0") != 0) {  
                wait(&status);
            }
        }
    } else {
        //printf("\nBuilt in command found :%s path :%s", command_args[0], command_args[1]);
        if (strcmp(command_args[0], "cd\0") == 0) {
            ret = change_dir(command_args[1]);
        } else if (strcmp(command_args[0], "export\0") == 0) {
            ret = export_var(command_args[1]);
        } else if (strcmp(command_args[0], "exit\0") == 0) {
            kill(0, SIGKILL);
            exit(0);
        } else {
            printf("\nBuilt command [%s] not supported yet.\n", command_args[0]);
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

int main(int argc, char *argv[], char *envp[]) {
    char *command_line = NULL;
    command_struct command_list[10];
    int ret = 0, bytes_written = 0;
    //char buf[1];
    //write(1, "Hello World!!!", 14);
    //read(0, buf, 1);
    //write(1, buf, 1);
    //setpgid(0,0); //Need to set same id for all the processes.
    //printf("\nenvp got :%s", envp[0]);
    if(argc > 1) {
        non_interactive(argv[1], command_list);
    }
    else {  
        do {
            bytes_written = write(STDOUT_FILENO, "\n", 1);
            bytes_written = write(STDOUT_FILENO, prompt, strlen(prompt));
            bytes_written = write(STDOUT_FILENO, ">", 1);
            if (bytes_written == 0) {
                break;
            }
            command_line = NULL;
            ret = get_command(&command_line);
            ret = split_command(command_line, command_list);
            ret = execute_command(command_list[0]);
            if (ret == -1) { //dead code to avoid set but not used error.
                break;
            }
        } while(strcmp(command_line,"exit\0") != 0);
    }

    if (command_line != NULL) {
        free(command_line);
    }
    //kill(0, SIGKILL);
    return 0;
}
