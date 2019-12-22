#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>

#define true 1
#define false 0
#define BUFSIZE 1024

int process_status;
int bufsize = BUFSIZE;

int SPACE = ' ';
int COLON = ':';
char** path_args;

void print_prompt();
void execute_args(char** args);
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
        args = split(line, SPACE);
        printf("Received %s\n", args[0]);
        pid_t pid = fork();
        if(pid == 0 && args[0]){
            printf("Running exec on %s : %s\n", args[0], *args);
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

char **split(char* input, int delim){
    char **array = malloc(bufsize * sizeof(char*));
    input[strlen(input)] = ' ';
    char *delimptr = strchr(input, delim);
    if(!delimptr){
        array[0] = input;
        array[1] = NULL;
        free(delimptr);
        return array;
    }
    int i = 0;
    int beginindex = 0;
    int ptrindex = 0;
    while(delimptr != NULL){
        ptrindex = (int)(delimptr - input);
        char* token = malloc(ptrindex * sizeof(char));
        memcpy(token, input + beginindex, ptrindex - beginindex);
        printf("substring from %d to %d: %s\n", beginindex, ptrindex, token);
        array[i++] = token;
        printf("token: %s\n", token);
        delimptr = strchr(delimptr+1, delim);
        beginindex = ptrindex+1;
    }
    return array;
}