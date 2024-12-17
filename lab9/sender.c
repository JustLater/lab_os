#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#define BUFFER_SIZE 100

const char *shm_name = "shared_memory";
int shmid;
char *shared_memory = NULL;
int semid;

void handler(int sig) {
    printf("[SIGNAL HANDLER] Signal %d received\n", sig);
    if (shared_memory != NULL) {
        if (shmdt(shared_memory) < 0) {
            perror("shmdt");
        }
    }
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        perror("shmctl");
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl");
    }
    unlink(shm_name);
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
    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    key_t shm_key = ftok(shm_name, 'R');
    shmid = shmget(shm_key, BUFFER_SIZE, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid < 0) {
        perror("Another sender is already running or failed to create shared memory.");
        exit(1);
    }

    key_t sem_key = ftok(shm_name, 'S');
    semid = semget(sem_key, 1, IPC_CREAT | IPC_EXCL | 0666);
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
