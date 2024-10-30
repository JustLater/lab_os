#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

void print_usage() {
    printf("Использование: mychmod [options] file\n");
    printf("Example options:\n");
    printf("  +x           - добавляет права на выполнение\n");
    printf("  u-r          - удаляет права на чтение для владельца\n");
    printf("  g+rw         - добавляет права на чтение и запись для группы\n");
    printf("  766          - устанавливает конкретные права доступа\n");
}

int change_permissions(const char* path, const char* mode) {
    struct stat st;
    if (stat(path, &st) != 0) {
        perror("Ошибка получения информации о файле");
        return -1;
    }

    mode_t new_mode = st.st_mode;

    // Обработка символов chmod
    if (mode[0] == 'u' || mode[0] == 'g' || mode[0] == 'o') {
        char operation = mode[1];
        char permission = mode[2];
        mode_t mask = 0;

        if (permission == 'r') mask = S_IRUSR | S_IRGRP | S_IROTH;
        else if (permission == 'w') mask = S_IWUSR | S_IWGRP | S_IWOTH;
        else if (permission == 'x') mask = S_IXUSR | S_IXGRP | S_IXOTH;
        else {
            fprintf(stderr, "Ошибка: Неизвестный символ разрешения: %c\n", permission);
            return -1;
        }

        if (operation == '+') new_mode |= mask;
        else if (operation == '-') new_mode &= ~mask;
        else {
            fprintf(stderr, "Ошибка: Неизвестная операция: %c\n", operation);
            return -1;
        }
    } else {
        // Обработка числового режима
        new_mode = strtol(mode, NULL, 8);
    }

    // Установка новых прав
    if (chmod(path, new_mode) != 0) {
        perror("Ошибка изменения прав");
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        print_usage();
        return EXIT_FAILURE;
    }

    const char* mode = argv[1];
    const char* path = argv[2];

    if (change_permissions(path, mode) != 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
