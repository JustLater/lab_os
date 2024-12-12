#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define LIMIT 10

int array[LIMIT];
int next_write = 0;
pthread_rwlock_t rwlock;

void* write_thread() {
    for (int i = 0; i < LIMIT; i++) {
        usleep(10000);
        pthread_rwlock_wrlock(&rwlock);
        array[next_write] = next_write;
        printf("Written: %d\n", next_write);
        next_write++;
        pthread_rwlock_unlock(&rwlock);
    }
    return NULL;
}

void* read_thread(void* arg) {
    while (1) {
        usleep(5000);
        pthread_rwlock_rdlock(&rwlock);
        if (next_write > 0) {
            printf("Read: array[%d] = %d tid: %lx\n", next_write - 1, array[next_write - 1], pthread_self());
        }
        pthread_rwlock_unlock(&rwlock);
        if (next_write >= LIMIT) {
            break;
        }
    }
    return NULL;
}

int main() {
    pthread_rwlock_init(&rwlock, NULL);
    pthread_t writing_array;
    pthread_t reading_threads[LIMIT];

    pthread_create(&writing_array, NULL, write_thread, NULL);

    for (int i = 0; i < LIMIT; i++) {
        pthread_create(&reading_threads[i], NULL, read_thread, NULL);
    }

    pthread_join(writing_array, NULL);

    for (int i = 0; i < LIMIT; i++) {
        pthread_join(reading_threads[i], NULL);
    }

    pthread_rwlock_destroy(&rwlock);
    return 0;
}
