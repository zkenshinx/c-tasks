#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define EXPECTED_ARG_COUNT 5

void print_error_and_exit(char* error_message);
int run_prog1(char* prog);
int run_prog2(char* prog, int fd_pipe[]);
int run_prog3(char* prog, int fd_pipe[], int output_fd);

int main(int argc, char* argv[]) {
    if (argc != EXPECTED_ARG_COUNT) {
        fprintf(stderr, "Usage: program_name prog1 prog2 prog3 file");
        exit(EXIT_FAILURE);
    } 
    
    /*
        My idea is to redirect the stdout output of prog2
        to fd_pipe that will be read by prog3.
    */
    int fd_pipe[2];
    pipe(fd_pipe);

    // No matter the outcome (whether prog1 fails and so on..) file is created
    int output_fd = open(argv[4], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

    int prog1_status = run_prog1(argv[1]);
    if (prog1_status != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    run_prog2(argv[2], fd_pipe);
    close(fd_pipe[1]);

    // Even though prog2 might exit with failure, it still redirects it's stdout to prog3
    int prog3_status = run_prog3(argv[3], fd_pipe, output_fd);
    close(fd_pipe[0]);
    return prog3_status;
}

int run_prog1(char* prog) {
    int prog_status;
    int pid = fork();
    if (pid == 0) {
        char** args = malloc(2 * sizeof(char*));
        args[0] = prog;
        args[1] = NULL;

        execv(prog, args);
    } else if (pid > 0) {
        wait(&prog_status);
    } else {
        fprintf(stderr, "Forking failed");
        exit(EXIT_FAILURE);
    }
    return prog_status;
}

int run_prog2(char* prog, int fd_pipe[]) {
    int prog_status;
    int pid = fork();
    if (pid == 0) {
        dup2(fd_pipe[1], STDOUT_FILENO); // Replace stdout with write pipe
        char** args = malloc(2 * sizeof(char*));
        args[0] = prog;
        args[1] = NULL;

        execv(prog, args);
    } else if (pid > 0) {
        wait(&prog_status);
    } else {
        fprintf(stderr, "Forking failed");
        exit(EXIT_FAILURE);
    }
    return prog_status;
}

int run_prog3(char* prog, int fd_pipe[], int output_fd) {
    if (output_fd == -1) {
        fprintf(stderr, "Error while trying to open file");
        exit(EXIT_FAILURE);
    }

    int prog_status;
    int pid = fork();
    if (pid == 0) {
        
        dup2(fd_pipe[0], STDIN_FILENO); // Replace stdin with read pipe
        dup2(output_fd, STDOUT_FILENO); // Replace stdout with given file

        char** args = malloc(2 * sizeof(char*));
        args[0] = prog;
        args[1] = NULL;

        execv(prog, args);
    } else if (pid > 0) {
        wait(&prog_status);
    } else {
        fprintf(stdout, "Forking failed");
        exit(EXIT_FAILURE);
    }
    return prog_status;
}
