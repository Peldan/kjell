#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#define true 1
#define false 0
#define BUFSIZE 1024

int process_status;
int bufsize = BUFSIZE;

const int SPACE = ' ';
const int COLON = ':';
const int SEMICOLON = ';';

char** path_args;

void print_prompt();
void execute_args(char** args);
char** split(char* input, const int delim);
char* get_user_input(void);
int main();
void parse_path_args();

int main() {
    parse_path_args();
    char* line;
    char** args;
    do {
        print_prompt();
        line = get_user_input();
        args = split(line, SPACE);
        pid_t pid = fork();
        if(pid == 0 && args[0]){
            execute_args(args);
        }
        waitpid(pid, &process_status, 0);
        free(line);
        free(args);
    } while (!feof(stdin));
    free(path_args);
    exit(0);
}

void parse_path_args(){
    char* env;
    char* path = getenv("PATH");
    env = malloc(strlen(path) + 1);
    strcpy(env, getenv("PATH"));
    path_args = split(env, COLON);
}

void print_prompt(){
    char tmpbuf[BUFSIZE];
    char *cwd = getcwd(tmpbuf, sizeof(tmpbuf));
    printf("%s $ ", cwd);
    fflush(stdout);
}

void execute_args(char** args){
    printf("Tog emot %s för exekvering\n", args[0]);
    if(strcmp(args[0], "cd") == 0){
        chdir(args[1]);
    } else {
        execvp(args[0], args);
        printf("Oh dear, something went wrong! %s\n", strerror(errno));
    }
}

char *get_user_input(void){
    char* input = malloc(sizeof(char) * 64);
    int i = 0;
    while(true){
        char c = (char)getchar();
        if(c == EOF || c == '\n'){
            return input;
        }
        input[i] = c;
        i++;
    }
}

char *get_next_cmd(char* str){
    int i = 0;
    while(str[i] == ' '){
        str[i]++;
    }
    return str;
}

char **split(char* input, int delim){
    char** cmd_array = malloc(bufsize * sizeof(char*));
    char* cmd = get_next_cmd(input);
    char* delimptr = strchr(input, delim);
    int i = 0;
    while(delimptr != NULL){
        delimptr[0] = '\0';
        cmd_array[i++] = cmd;
        printf("token: %s\n", cmd);
        cmd = get_next_cmd(delimptr+1);
        delimptr = strchr(cmd, delim);
    }
    if(!isspace(cmd[0])){
        cmd_array[i] = cmd;
        ++i;
    }
    cmd_array[i] = NULL;
    return cmd_array;
}