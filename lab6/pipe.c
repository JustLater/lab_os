#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 256

void print_time(const char *process_name) {
    time_t current_time;
    char time_string[64];
    struct tm *tm_info;

    time(&current_time);
    tm_info = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%a %b %d %T %Y", tm_info);
    printf("Time in %s process: %s\n", process_name, time_string);
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    int pipe_fd[2];
    char buffer[BUF_SIZE];

    if (pipe(pipe_fd) == -1) {
        perror("Error creating pipe");
        exit(EXIT_FAILURE);
    }

    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("Error during fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        close(pipe_fd[1]);
        print_time("CHILD");

        if (read(pipe_fd[0], buffer, BUF_SIZE) > 0) {
            printf("%s", buffer);
        }
        close(pipe_fd[0]);
    } else {
        close(pipe_fd[0]);
        sleep(5);

        print_time("PARENT");
        snprintf(buffer, BUF_SIZE, "PARENT pid = %d\n", getpid());

        if (write(pipe_fd[1], buffer, strlen(buffer) + 1) == -1) {
            perror("Error writing to pipe");
            close(pipe_fd[1]);
            exit(EXIT_FAILURE);
        }
        close(pipe_fd[1]);
        wait(NULL);
    }

    return 0;
}