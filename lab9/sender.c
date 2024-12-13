#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#define SHM_KEY 1235
#define SEM_KEY 5677
#define BUFFER_SIZE 100

int shmid;
char *shared_memory;
int semid;

void cleanup(int signum) {
    if (shmid >= 0) {
        shmctl(shmid, IPC_RMID, NULL);
    }
    if (semid >= 0) {
        semctl(semid, 0, IPC_RMID);
    }
    exit(0);
}

void semaphore_wait(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void semaphore_signal(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = cleanup;
    sigaction(SIGINT, &sa, NULL);

    shmid = shmget(SHM_KEY, BUFFER_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid < 0) {
        perror("Another sender is already running.");
        exit(1);
    }

    semid = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid < 0) {
        perror("Failed to create semaphore.");
        exit(1);
    }

    if (semctl(semid, 0, SETVAL, 1) < 0) {
        perror("Failed to initialize semaphore.");
        exit(1);
    }

    shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    time_t current_time;
    while (1) {
        semaphore_wait(semid);
        current_time = time(NULL);
        pid_t pid = getpid();
        snprintf(shared_memory, BUFFER_SIZE, "Time: %sPID: %d\n", ctime(&current_time), pid);
        semaphore_signal(semid);
        sleep(1);
    }

    return 0;
}
