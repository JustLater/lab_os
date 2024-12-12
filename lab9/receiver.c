#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY 1234
#define SEM_KEY 5678
#define BUFFER_SIZE 100

int shmid;
char *shared_memory;
int semid;

void cleanup(int signum) {
    shmdt(shared_memory);
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

    shmid = shmget(SHM_KEY, BUFFER_SIZE, 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    semid = semget(SEM_KEY, 1, 0666);
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
