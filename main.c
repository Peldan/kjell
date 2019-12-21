#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define true 1
#define false 0
#define bool int
#define BUFSIZE 1024

bool running;
int bufsize = BUFSIZE;

char** split(char* input);
char* getuserinput(void);
int main();

int main() {
    running = true;
    char* line;
    char** args;
    do {
        printf("$ ");
        fflush(stdout);
        line = getuserinput();
        args = split(line);
        pid_t pid = fork();
        if(pid == 0){
            printf("pid 0.. executing %s -> %s\n", args[0], *args);
            execvp(args[0], args);
        }
        int status;
        waitpid(pid, &status, 0);
        free(line);
        free(args); //ska vara sist i metoden!
    } while(running);

    exit(0);
}

char *getuserinput(void){
    char* input = malloc(sizeof(char) * 64);
    scanf("%[^\n]s", input);
    return input;
}

char **split(char* input){
    char **args = malloc(bufsize * sizeof(char));
    char *arg;
    arg = strtok(input, " ");
    int i = 0;
    while(arg != NULL){
        args[i++] = arg;
        printf("Ord: %s\n", arg);
        arg = strtok(NULL, " ");
    }
    args[i] = NULL;
    return args;
}

