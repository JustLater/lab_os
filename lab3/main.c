#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

// Флаг, указывающий, что родительский процесс получил сигнал SIGINT
volatile sig_atomic_t parent_received_sigint = 0;

// Обработчик сигнала SIGINT
void sigint_handler(int sig) {
    printf("Received SIGINT signal. Terminating processes.\n");
    parent_received_sigint = 1;
    exit(1);
}

// Обработчик сигнала SIGTERM
void sigterm_handler(int sig, siginfo_t *info, void *context) {
    printf("Received SIGTERM signal from process %d. Terminating.\n", info->si_pid);
    exit(1);
}

// Функция, вызываемая при завершении процесса (для родительского процесса)
void parent_exit_handler(int status, void *arg) {
    printf("Parent process with PID %d has terminated.\n", getpid());
}

// Функция, вызываемая при завершении процесса (для дочернего процесса)
void child_exit_handler(int status, void *arg) {
    printf("Child process with PID %d has terminated.\n", getpid());
}

int main() {
    printf("Parent process with PID %d starts execution.\n", getpid());

    // Регистрируем обработчик для SIGINT
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        printf("Error registering SIGINT handler.\n");
        return 1;
    }

    // Регистрируем обработчик для SIGTERM
    struct sigaction sa;
    sa.sa_sigaction = sigterm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGTERM, &sa, NULL) != 0) {
        printf("Error registering SIGTERM handler.\n");
        return 1;
    }

    // Регистрируем обработчик для родительского процесса
    if (atexit(parent_exit_handler) != 0) {
        printf("Error registering parent exit handler.\n");
        return 1;
    }

    // Вызываем fork() для создания дочернего процесса
    pid_t child_pid = fork();

    if (child_pid == -1) { // Произошла ошибка при вызове fork()
        printf("Error creating child process.\n");
        return 1;
    } else if (child_pid == 0) { // Это дочерний процесс
        // Регистрируем обработчик для дочернего процесса
        if (on_exit(child_exit_handler, NULL) != 0) {
            printf("Error registering child exit handler.\n");
            return 1;
        }

        printf("Child process with PID %d starts execution.\n", getpid());
        // Здесь можно выполнить некоторые действия, специфичные для дочернего процесса
        printf("Child process terminates.\n");
        return 0;
    } else { // Это родительский процесс
        printf("Parent process with PID %d waits for child process to finish.\n", getpid());
        // Ожидаем завершения дочернего процесса
        int status;
        waitpid(child_pid, &status, 0);
        printf("Child process with PID %d has finished.\n", child_pid);

        // Если родительский процесс получил сигнал SIGINT, завершаем программу
        if (parent_received_sigint) {
            printf("Parent process terminating due to SIGINT.\n");
            return 1;
        }

        return 0;
    }
}




