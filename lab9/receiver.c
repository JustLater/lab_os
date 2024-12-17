#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define BUFFER_SIZE 100

const char *shm_name = "shared_memory";
int shmid;
char *shared_memory = NULL;
int semid;

void handler(int signal) {
    printf("[SIGNAL HANDLER] Signal %d received\n", signal);
    if (shared_memory != NULL) {
        if (shmdt(shared_memory) < 0) {
            perror("shmdt");
        }
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
    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    key_t shm_key = ftok(shm_name, 'R');
    shmid = shmget(shm_key, BUFFER_SIZE, 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    key_t sem_key = ftok(shm_name, 'S');
    semid = semget(sem_key, 1, 0666);
    if (semid < 0) {
        perror("Failed to get semaphore.");
        exit(1);
    }

    shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    while (1) {
        semaphore_wait(semid);
        printf("Receiver PID: %d | Received: %s", getpid(), shared_memory);
        semaphore_signal(semid);
        sleep(1);
    }

    return 0;
}
