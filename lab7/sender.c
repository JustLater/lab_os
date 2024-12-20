#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h>

#define BUFFER_SIZE 100

int shmid;
char *shared_memory = NULL;
const char *shm_name = "ftok_file";

void cleanup(int sig) {
    printf("[SIGNAL HANDLER] Signal %d received\n", sig);

    if (shared_memory != NULL && shmdt(shared_memory) < 0) {
        int err = errno;
        fprintf(stderr, "In shmdt %s (%d)\n", strerror(err), err);
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        int err = errno;
        fprintf(stderr, "In shmctl %s (%d)\n", strerror(err), err);
        exit(1);
    }

    unlink(shm_name);
    exit(0);
}


int main() {

    FILE *file = fopen(shm_name, "w");
    if (file) {
        fclose(file);
    }
    else {
        perror("Failed to create the file");
        exit(1);
    }

    // Генерация ключа для разделяемой памяти
    key_t key = ftok(shm_name, 'R');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    // Проверка на существование разделяемой памяти
    shmid = shmget(key, BUFFER_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid < 0) {
        perror("Another sender is already running.");
        exit(1);
    }

    if (signal(SIGINT, cleanup) == SIG_ERR || signal(SIGTERM, cleanup) == SIG_ERR) {
        perror("Error in signal");
        return 1;
    }

    shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    time_t current_time;
    while (1) {
        current_time = time(NULL);
        pid_t pid = getpid();
        snprintf(shared_memory, BUFFER_SIZE, "Time: %sPID: %d\n", ctime(&current_time), pid);
        sleep(1);
    }
    return 0;
}
