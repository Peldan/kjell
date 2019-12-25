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
#define SUCCESS 0

//STRUCT

struct cmd {
    char **args;
};

//VARIABLES
int process_status;
int bufsize = BUFSIZE;

const char *SPACE = " ";
const char *COLON = ":";
const char *SEMICOLON = ";";
const char *PIPE = "|";
const char *AND = "&&";

char **path_args;

//FUNC PROTOTYPES
void handle_command(char *line);

void print_prompt();

void execute_args(struct cmd commands[], const char *delim, int cmd_amount);

void execute_pipe(char **from, char **to);

void handle_special_characters(char *input);

char **strsplit(char *input, const char *delim);

char *get_user_input(void);

int main();

void parse_path_args();

void execute_chained(const struct cmd *commands, const int cmd_amount);

int main() {
    parse_path_args();
    char *line;
    do {
        print_prompt();
        line = get_user_input();
        if (strlen(line) > 0) {
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
    env = calloc(1, strlen(path) + 1);
    strcpy(env, path);
    path_args = strsplit(env, COLON);
    free(env);
    env = NULL;
}

void print_prompt() {
    char tmpbuf[BUFSIZE];
    char *cwd = getcwd(tmpbuf, sizeof(tmpbuf));
    printf("%s $ ", cwd);
    fflush(stdout);
}

void execute_args(struct cmd commands[CMD_MAX], const char *delim, const int cmd_amount) {
    if (delim == SEMICOLON) {
        execute_chained(commands, cmd_amount);
    }
    else if (delim == PIPE) {
        execute_pipe(commands[0].args, commands[1].args);
    }
    else if (delim == AND) {

    }
}

void execute_chained(const struct cmd *commands, const int cmd_amount) {
    for (int i = 0; i < cmd_amount; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char **cmd_arr = commands[i].args;
            while (*cmd_arr != NULL) {
                execvp(*cmd_arr, cmd_arr);
                printf("Oh dear, something went wrong! %s\n", strerror(errno));
                cmd_arr++;
            }
        }
        waitpid(pid, &process_status, 0);
        printf("\n");
    }
}

void execute_pipe(char **from, char **to) {
    int pipefd[2];
    pid_t child;
    if(pipe(pipefd) != 0){
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    child = fork();
    if(child == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if(child == SUCCESS){
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[STDOUT_FILENO]);
        execvp(*from, from);
        waitpid(child, &process_status, 0);
    } else {
        child = fork();
        if(child == 0){
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[STDIN_FILENO]);
            execvp(*to, to);
        } else {
            close(pipefd[STDIN_FILENO]);
            close(pipefd[STDOUT_FILENO]);
            waitpid(child, &process_status, 0);
        }
    }
}

void execute_single_arg(struct cmd command) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(*command.args, command.args);
        printf("Oh dear, something went wrong! %s\n", strerror(errno));
    }
    waitpid(pid, &process_status, 0);
}

char *get_user_input(void) {
    char *input = calloc(1, sizeof(char) * 64);
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

char *skip_whitespace_cmd(char *str) {
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

    if (strstr(input, AND) == NULL && strstr(input, SEMICOLON) == NULL && strstr(input, PIPE) == NULL) {
        struct cmd command = {strsplit(input, SPACE)};
        execute_single_arg(command);
        return;
    }

    for (int i = 0; i < len; i++) {
        int cmdnum = 0;
        struct cmd commands[CMD_MAX];
        char *delimptr = strstr(input, special_characters[i]);
        char *cmd = skip_whitespace_cmd(input);
        strcat(input, special_characters[i]);
        while (delimptr != NULL) {
            delimptr[0] = '\0'; //removes the delimiter character
            if (special_characters[i] == AND) {
                delimptr[1] = '\0';
            }
            char *before_delim = calloc(1, strlen(input) * sizeof(char));
            strncpy(before_delim, cmd, (delimptr - input));
            char **split_cmd = strsplit(before_delim, SPACE);
            commands[cmdnum++].args = split_cmd;
            cmd = skip_whitespace_cmd(delimptr + 1);
            delimptr = strstr(cmd, SEMICOLON);
        }
        execute_args(commands, special_characters[i], cmdnum);
    }
}

char **strsplit(char *input, const char *delim) {
    char **cmd_array = calloc(1, bufsize * sizeof(char *));
    char *cmd = skip_whitespace_cmd(input);
    char *delimptr = strstr(input, delim);
    int i = 0;
    while (delimptr != NULL) {
        delimptr[0] = '\0';
        cmd_array[i++] = cmd;
        cmd = skip_whitespace_cmd(delimptr + 1);
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