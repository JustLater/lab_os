#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

int is_valid_permission(const char *mode) {
    if (strlen(mode) == 0) return 0;

    int has_symbolic = 0;
    int has_numeric = 0;

    // Проверка ввода
    for (int i = 0; mode[i] != '\0'; i++) {
        if (mode[i] >= '0' && mode[i] <= '7') {
            has_numeric = 1;
        } else if (strchr("ugoa+-,rwx", mode[i]) != NULL) {
            has_symbolic = 1;
        } else {
            return 0;
        }
    }

    // Проверка на одновременное использование чисел и символов
    if (has_numeric && has_symbolic) {
        return 0;
    }

    return 1;
}

int change_permissions(const char* path, const char* mode) {
    struct stat st;
    if (stat(path, &st) != 0) {
        perror("Ошибка получения информации о файле");
        return -1;
    }

    mode_t new_mode = st.st_mode;

    // Проверка на валидность
    if (!is_valid_permission(mode)) {
        fprintf(stderr, "Ошибка: Неизвестная команда или неправильный формат: %s\n", mode);
        return -1;
    }

    // Проверка на числовой режим
    if (mode[0] >= '0' && mode[0] <= '7') {
        new_mode = strtol(mode, NULL, 8); // Преобразование в числовой режим
    } else {
        // Обработка символов chmod
        const char *ptr = mode;

        while (*ptr) {
            char who = *ptr++;
            char operation;

            if (who == '+' || who == '-') { //упрощена обработка операций и кто
                operation = who;
                who = 'a';
            } else {
                operation = *ptr++;
            }

            // Обработка разрешений
            mode_t mask = 0; // Маска для установки прав вынесена за цикл, чтобы не сбрасывать ее при каждой итерации
            //и в цикле вместо = теперь |= (чтобы не сбрасывать права)
            while (*ptr && (*ptr == 'r' || *ptr == 'w' || *ptr == 'x')) {
                switch (who) {
                    case 'u':
                        if (*ptr == 'r') mask |= S_IRUSR;
                        if (*ptr == 'w') mask |= S_IWUSR;
                        if (*ptr == 'x') mask |= S_IXUSR;
                        break;
                    case 'g':
                        if (*ptr == 'r') mask |= S_IRGRP;
                        if (*ptr == 'w') mask |= S_IWGRP;
                        if (*ptr == 'x') mask |= S_IXGRP;
                        break;
                    case 'o':
                        if (*ptr == 'r') mask |= S_IROTH;
                        if (*ptr == 'w') mask |= S_IWOTH;
                        if (*ptr == 'x') mask |= S_IXOTH;
                        break;
                    case 'a':
                        if (*ptr == 'r') mask |= S_IRUSR | S_IRGRP | S_IROTH;
                        if (*ptr == 'w') mask |= S_IWUSR | S_IWGRP | S_IWOTH;
                        if (*ptr == 'x') mask |= S_IXUSR | S_IXGRP | S_IXOTH;
                        break;
                }
                ptr++;
            }

            if (operation == '+') new_mode |= mask;
            else if (operation == '-') new_mode &= ~mask;
        }
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
        fprintf(stderr, "Использование: mychmod [mode] file\n");
        return EXIT_FAILURE;
    }
    const char* mode = argv[1];
    const char* path = argv[2];
    if (change_permissions(path, mode) != 0) {
        return EXIT_FAILURE; // Возвращаем ошибку, если изменение прав не произошло
    }
    return EXIT_SUCCESS;
}
