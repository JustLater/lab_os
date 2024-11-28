#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>

#define BUF_SIZE 256

void get_current_time(char *time_str, size_t size) {
    time_t current_time;
    struct tm *tm_info;

    time(&current_time);
    tm_info = localtime(&current_time);
    strftime(time_str, size, "%a %b %d %T %Y", tm_info);
}

int create_fifo(const char *fifoname) {
    if (mkfifo(fifoname, 0600) == -1) { // Права доступа на чтение и запись только для влд
        perror("Error creating FIFO");
        return -1;
    }
    return 0;
}

// Проверка на существование
int check_fifo_exists(const char *fifoname) {
    struct stat st;
    if (stat(fifoname, &st) == 0) {
        fprintf(stderr, "File %s already exists\n", fifoname);
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "FIFO name not specified.\n");
        return EXIT_FAILURE;
    }

    const char *fifoname = argv[1];

    if (check_fifo_exists(fifoname)) {
        return EXIT_FAILURE;
    }

    if (create_fifo(fifoname) == -1) {
        return EXIT_FAILURE;
    }

    char buffer[BUF_SIZE];
    char time_str[64];
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("Error during fork");
        unlink(fifoname);
        return EXIT_FAILURE;
    }

    if (child_pid == 0) {
        int fd = open(fifoname, O_RDONLY);
        if (fd == -1) {
            perror("Error opening FIFO for reading");
            unlink(fifoname); // Удаляем FIFO
            return EXIT_FAILURE;
        }

        get_current_time(time_str, sizeof(time_str));
        printf("Time in CHILD process: %s\n", time_str);

        if (read(fd, buffer, BUF_SIZE) == -1) {
            perror("Error reading from FIFO");
            close(fd);
            unlink(fifoname);
            return EXIT_FAILURE;
        }

        printf("%s", buffer);

        if (close(fd) == -1) {
            perror("Error closing FIFO");
            unlink(fifoname);
            return EXIT_FAILURE;
        }

    } else {
        int fd = open(fifoname, O_WRONLY);
        if (fd == -1) {
            perror("Error opening FIFO for writing");
            unlink(fifoname);
            return EXIT_FAILURE;
        }

        sleep(5);

        get_current_time(time_str, sizeof(time_str));
        snprintf(buffer, BUF_SIZE, "Time in PARENT process: %s\nPARENT pid = %d\n", time_str, getpid());

        if (write(fd, buffer, strlen(buffer) + 1) == -1) {

            perror("Error writing to FIFO");
            close(fd);
            unlink(fifoname);
            return EXIT_FAILURE;
        }

        if (close(fd) == -1) {
            perror("Error closing FIFO");
            unlink(fifoname);
            return EXIT_FAILURE;
        }

        wait(NULL);

        if (unlink(fifoname) == -1) {
            perror("Error deleting FIFO");
            return EXIT_FAILURE;
        }
    }

    return 0;
}
