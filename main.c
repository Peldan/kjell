#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <ctype.h>

#include <sys/wait.h>

#define true 1
#define false 0
#define bool int
#define BUFSIZE 1024
#define CMD_MAX 10

int process_status;
int bufsize = BUFSIZE;

const char *SPACE = " ";
const char *COLON = ":";
const char *SEMICOLON = ";";
const char *PIPE = "|";
const char *AND = "&&";

char **path_args;

void handle_command(char *line);

void print_prompt();

void execute_args(char **args[], const char *delim);

void handle_special_characters(char *input);

char **split(char *input, const char *delim);

char *get_user_input(void);

int main();

void parse_path_args();

int main() {
    parse_path_args();
    char *line;
    do {
        print_prompt();
        line = get_user_input();
        if(strlen(line) > 0){
            handle_command(line);
        }
        free(line);
    } while (!feof(stdin));
    free(path_args);
    free(line);
    exit(0);
}

void handle_command(char *line) {
    handle_special_characters(line);
}

void parse_path_args() {
    char *env;
    char *path = getenv("PATH");
    env = malloc(strlen(path) + 1);
    strcpy(env, path);
    path_args = split(env, COLON);
    free(env);
    env = NULL;
}

void print_prompt() {
    char tmpbuf[BUFSIZE];
    char *cwd = getcwd(tmpbuf, sizeof(tmpbuf));
    printf("%s $ ", cwd);
    fflush(stdout);
}

void execute_args(char **args[], const char *delim) {
    pid_t pid = fork();
    if (pid == 0) {
        for (char **c = *args; c; c = *++args) {
            if (strcmp(c[0], "cd") == 0) {
                chdir(c[1]);
            } else {
                printf("received %s --- %s for exec\n",c[0], *c);
                if (delim == PIPE) {
                    printf("\n\nRUNNING PIPE\n\n");
                    printf("Oh dear, something went wrong! %s\n", strerror(errno));
                } else if (delim == AND) {
                    printf("\n\nRUNNING &&\n\n");
                    if (execvp(c[0], c) == -1) {
                        printf("fel: %s\n", strerror(errno));
                        break;
                    }
                    execvp(c[0], c);
                    printf("ran %s - %s\n", c[0], *c);
                } else {
                    execvp(c[0], c);
                }
                printf("Oh dear, something went wrong! %s\n", strerror(errno));
            }
        }
    }
    waitpid(pid, &process_status, 0);
}

char *get_user_input(void) {
    char *input = calloc(sizeof(char) * 64, sizeof(char));
    int i = 0;
    while (true) {
        char c = (char) getchar();
        if (c == EOF || c == '\n') {
            input[i] = '\0';
            return input;
        }
        input[i] = c;
        i++;
    }
}

void remove_trailing_whitespace(char *str) {
    unsigned long i = strlen(str) - 1;
    while (str[i] == ' ') {
        str[i] = '\0';
        i--;
    }
}

char *get_next_cmd(char *str) {
    int i = 0;
    while (str[i] == ' ') {
        i++;
    }
    remove_trailing_whitespace(str);
    return &str[i];
}

void handle_special_characters(char *input) {
    const char *special_characters[] = {SEMICOLON, PIPE, AND};
    size_t len = sizeof(special_characters) / sizeof(special_characters[0]);
    for (int i = 0; i < len; i++) {
        char **to_exec[CMD_MAX];
        char *delimptr = strstr(input, special_characters[i]);
        char *cmd = get_next_cmd(input);
        if (delimptr == NULL) {
            if(i == len - 1){
                char **split_cmd = split(cmd, SPACE);
                to_exec[0] = split_cmd;
                execute_args(to_exec, SPACE);
                free(split_cmd);
                split_cmd = NULL;
            }
            continue;
        }
        strcat(input, special_characters[i]);
        while (delimptr != NULL) {
            delimptr[0] = '\0'; //removes the delimiter character
            if (special_characters[i] == AND) {
                delimptr[1] = '\0';
            }
            char *before_delim = malloc(strlen(cmd) * sizeof(char));
            strncpy(before_delim, cmd, (delimptr - input));
            char **split_cmd = split(before_delim, SPACE);
            to_exec[i] = split_cmd;
            cmd = get_next_cmd(delimptr + 1);
            delimptr = strstr(cmd, SEMICOLON);
            free(split_cmd);
            split_cmd = NULL;
        }
        execute_args(to_exec, special_characters[i]);
    }
}

char **split(char *input, const char *delim) {
    char **cmd_array = malloc(bufsize * sizeof(char *));
    char *cmd = get_next_cmd(input);
    char *delimptr = strstr(input, delim);
    int i = 0;
    while (delimptr != NULL) {
        delimptr[0] = '\0';
        cmd_array[i++] = cmd;
        cmd = get_next_cmd(delimptr + 1);
        delimptr = strstr(cmd, delim);
    }
    free(delimptr);
    if (!isspace(cmd[0])) {
        cmd_array[i] = cmd;
        ++i;
    }
    cmd_array[i] = NULL;
    return cmd_array;
}