#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define LIMIT 10

int array[1000];
int next_write = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;

void* write_thread() {
    for (int i = 0; i < LIMIT; i++) {
        usleep(10000);
        pthread_mutex_lock(&mutex);
        array[next_write] = next_write;

        printf("Written: %d\n", next_write);

        next_write++;
        usleep(10000);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* read_thread() {
    usleep(10000);
    for (int i = 0; i < LIMIT; i++) {
        usleep(10000);
        pthread_mutex_lock(&mutex);
        while (i >= next_write) {
            pthread_cond_wait(&cond, &mutex);
        }

        printf("Read: array[%d] = %d tid: %lx\n", i, array[i], pthread_self());

        pthread_mutex_unlock(&mutex);
        usleep(10000);
    }
    return NULL;
}

int main() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
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

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
