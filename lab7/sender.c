#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define SHM_KEY 1234
#define BUFFER_SIZE 100

int shmid;
char *shared_memory;

void cleanup(int signum) {
    if (shmid >= 0) {
        shmctl(shmid, IPC_RMID, NULL);
    }
    exit(0);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = cleanup;
    sigaction(SIGINT, &sa, NULL);

    // Проверка на существование разделяемой памяти
    shmid = shmget(SHM_KEY, BUFFER_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid < 0) {
        perror("Another sender is already running.");
        exit(1);
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



