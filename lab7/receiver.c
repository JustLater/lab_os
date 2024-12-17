#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define BUFFER_SIZE 100

int shmid;
char *shared_memory;
const char *filename = "ftok_file";

int main() {

    // Генерация ключа для разделяемой памяти
    key_t key = ftok(filename, 'R');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    shmid = shmget(key, BUFFER_SIZE, 0666);
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
