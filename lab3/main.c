#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

// Обработчик для atexit
void func() {
    printf("I'm atexit 1 for process %d\n", getpid());
}

// Обработчик сигналов
void handler(int sig) {
    switch(sig) {
        case SIGTERM:
            printf("Signal SIGTERM received, my pid is %d\n", getpid());
            break;
        case SIGINT:
            printf("Signal SIGINT received, my pid is %d\n", getpid());
            break;
        default:
            printf("Signal %d received, my pid is %d\n", sig, getpid());
            break;
    }
}

int main(int argc, char** argv) {

    (void)argc; (void)argv;

    // Установка обработчиков сигналов
    if (signal(SIGINT, handler) == SIG_ERR) {
        perror("Error signal\n");
    }

    struct sigaction sigTermAction;
    sigTermAction.sa_handler = handler;
    sigTermAction.sa_flags = SA_SIGINFO;
    if (sigaction(SIGTERM, &sigTermAction, NULL) == -1) {
        perror("Error sigaction\n");
    }

    // Регистрация функций для вызова при завершении программы
    if (atexit(func) != 0) {
        perror("Atexit unsuccess\n");
    }

    pid_t res = 0;

    switch (res = fork()) {
        case -1: {
            // Обработка ошибки fork()
            int err = errno;
            fprintf(stderr, "Fork error: %s (%d)\n", strerror(err), err);
            break;
        }
        case 0: {
            // В дочернем процессе
            printf("[CHILD] I'm child of %d, my pid is %d\n", getppid(), getpid());
            break; // Завершаем дочерний процесс
        }
        default: {
            // В родительском процессе
            int ch_res;
            sleep(5);
            wait(&ch_res); // Ожидание завершения дочернего процесса
            printf("[PARENT] I'm parent of %d, my pid is %d, my parent pid is %d\n", res, getpid(), getppid());
            printf("[PARENT] Child exit code %d\n", WEXITSTATUS(ch_res));
            break;
        }
    }
    return 0;
}
