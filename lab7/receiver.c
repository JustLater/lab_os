#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>

#define SHM_KEY 1235
#define BUFFER_SIZE 100

int shmid;
char *shared_memory;

void cleanup(int signum) {
    shmdt(shared_memory);
    exit(0);
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

    shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    while (1) {
        printf("Receiver PID: %d | Received: %s", getpid(), shared_memory);
        sleep(1);
    }

    return 0;
}

