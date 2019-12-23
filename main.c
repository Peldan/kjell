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
void handle_special_characters(char* input);
char** split(char* input, int delim);
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
        handle_special_characters(line);
        free(line);
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
    pid_t pid = fork();
    if(pid == 0 && args[0]){
        printf("Tog emot %s för exekvering\n", args[0]);
        if(strcmp(args[0], "cd") == 0){
            chdir(args[1]);
        } else {
            execvp(args[0], args);
            printf("Oh dear, something went wrong! %s\n", strerror(errno));
        }
    }
    waitpid(pid, &process_status, 0);
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

void remove_trailing_whitespace(char* str){
    unsigned long i = strlen(str) - 1;
    while(str[i] == ' '){
        str[i] = '\0';
        i++;
    }
}

char *get_next_cmd(char* str){
    int i = 0;
    while(str[i] == ' '){
        i++;
    }
    remove_trailing_whitespace(str);
    return &str[i];
}

void handle_special_characters(char* input){
    char* delimptr = strchr(input, SEMICOLON);
    char* cmd = get_next_cmd(input);
    if(delimptr == NULL){
        execute_args(split(input, SPACE));
        return;
    }
    input[strlen(input)] = ';'; //appends a delim to last index so that all commands are executed
    while(delimptr != NULL){
        delimptr[0] = '\0'; //removes the delimiter character
        char* before_delim = malloc(strlen(cmd)*sizeof(char));
        printf("delim index: %lu\n", delimptr - input);
        strncpy(before_delim, cmd, (delimptr - input));
        printf("before_delim: %s\n", before_delim);
        execute_args(split(before_delim, SPACE));
        cmd = get_next_cmd(delimptr+1);
        delimptr = strchr(cmd, SEMICOLON);
        free(before_delim);
    }
}

char **split(char* input, int delim){
    char** cmd_array = malloc(bufsize * sizeof(char*));
    char* cmd = get_next_cmd(input);
    char* delimptr = strchr(input, delim);
    int i = 0;
    while(delimptr != NULL){
        delimptr[0] = '\0';
        cmd_array[i++] = cmd;
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