#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define ARCH_HEADER_SIZE  sizeof(struct archive_entry) // Размер заголовка для одного файла в архиве
#define MAX_FILENAME_LEN   256 // Максимальная длина имени файла

// Структура для хранения информации о файлах в архиве
struct archive_entry {
    char filename[MAX_FILENAME_LEN]; // Имя файла
    mode_t mode;                     // Режим (права доступа)
    off_t size;                      // Размер файла
};

// Функция для вывода справки по использованию архиватора
void usage() {
    printf("Usage:\n");
    printf("  ./archiver arch_name -i(--input) file1 ... fileN - добавить файлы в архив\n");
    printf("  ./archiver arch_name -s(--stat) - показать состояние архива\n");
    printf("  ./archiver arch_name -e(--extract) file - извлечь и удалить из архива\n");
    printf("  ./archiver -h(--help) - показать это сообщение\n");
}

// Функция для проверки, существует ли файл с указанным именем в архиве
int file_exists_in_archive(int arch_fd, const char *file_name) {
    struct archive_entry entry;
    off_t current_pos = lseek(arch_fd, 0, SEEK_CUR); // Запоминаем текущее положение в архиве

    // Читаем архив и ищем файл
    while (read(arch_fd, &entry, ARCH_HEADER_SIZE) == ARCH_HEADER_SIZE) {
        // Сравниваем имена файлов
        if (strncmp(entry.filename, file_name, MAX_FILENAME_LEN) == 0) {
            lseek(arch_fd, current_pos, SEEK_SET); // Возвращаемся к исходной позиции
            return 1; // Файл существует в архиве
        }
        lseek(arch_fd, entry.size, SEEK_CUR); // Пропускаем содержимое файла
    }

    lseek(arch_fd, current_pos, SEEK_SET); // Возвращаемся к исходной позиции
    return 0; // Файл не найден
}

// Функция для добавления файла в архив
void add_file_to_archive(int arch_fd, const char *file_name) {
    // Проверяем, существует ли файл в архиве
    if (file_exists_in_archive(arch_fd, file_name)) {
        printf("Ошибка: Файл %s уже существует в архиве.\n", file_name);
        return; // Прерываем выполнение, если файл уже есть в архиве
    }

    struct archive_entry entry;
    int file_fd = open(file_name, O_RDONLY); // Открываем файл для чтения
    if (file_fd == -1) {
        perror("Ошибка открытия файла");
        return;
    }

    // Получаем информацию о файле
    struct stat file_stat;
    if (fstat(file_fd, &file_stat) == -1) {
        perror("Ошибка получения информации о файле");
        close(file_fd);
        return;
    }

    // Заполняем структуру archive_entry
    strncpy(entry.filename, file_name, MAX_FILENAME_LEN);
    entry.mode = file_stat.st_mode;
    entry.size = file_stat.st_size;

    // Записываем заголовок в архив
    write(arch_fd, &entry, ARCH_HEADER_SIZE);

    // Записываем содержимое файла в архив
    char buffer[4096]; // Буфер для чтения данных
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        write(arch_fd, buffer, bytes_read);
    }

    close(file_fd); // Закрываем файловый дескриптор
}

// Функция для показа состояния архива (имена файлов и их атрибуты)
void show_archive_stat(int arch_fd) {
    struct archive_entry entry;

    // Читаем каждый файл из архива и выводим его информацию
    while (read(arch_fd, &entry, ARCH_HEADER_SIZE) == ARCH_HEADER_SIZE) {

        printf("Файл: %s, Размер: %lld байт, Режим: %o\n",
               entry.filename, (long long)entry.size, entry.mode);
        lseek(arch_fd, entry.size, SEEK_CUR); // Пропускаем содержимое файла
    }
}

// Функция для извлечения файла из архива и удаления его из архива
void extract_file_from_archive(const char *arch_name, const char *file_name) {
    struct archive_entry entry;
    int arch_fd = open(arch_name, O_RDWR); // Открываем архив для чтения и записи
    if (arch_fd == -1) {
        perror("Ошибка открытия архива");
        return;
    }

    // Создаем временный файл для хранения новых данных
    int temp_fd = open("temp_archive", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (temp_fd == -1) {
        perror("Ошибка создания временного архива");
        close(arch_fd);
        return;
    }

    int found = 0; // Флаг для определения, извлекли ли мы файл

    // Читаем архив и ищем нужный файл
    while (read(arch_fd, &entry, ARCH_HEADER_SIZE) == ARCH_HEADER_SIZE) {
        // Записываем все файлы в новый архив, кроме извлекаемого
        if (strncmp(entry.filename, file_name, MAX_FILENAME_LEN) == 0) {
            found = 1; // Устанавливаем флаг, если нашли извлекаемый файл
            printf("Извлечение %s...\n", entry.filename);

            // Открываем файл для записи
            int out_fd = open(entry.filename, O_WRONLY | O_CREAT | O_TRUNC, entry.mode);
            if (out_fd == -1) {
                perror("Ошибка создания выходного файла");
                break; // Если возникла ошибка, выходим из цикла
            }

            // Записываем содержимое файла в выходной файл
            char buffer[4096];
            ssize_t bytes_read;
            off_t bytes_to_read = entry.size; // Количество байт для чтения

            while (bytes_to_read > 0 && (bytes_read = read(arch_fd, buffer, sizeof(buffer))) > 0) {
                write(out_fd, buffer, bytes_read);
                bytes_to_read -= bytes_read;
            }
            close(out_fd); // Закрываем выходной файл
        } else {
            // Если это не извлекаемый файл, то просто копируем его в новый архив
            write(temp_fd, &entry, ARCH_HEADER_SIZE);
            char *file_buffer = malloc(entry.size);
            read(arch_fd, file_buffer, entry.size);
            write(temp_fd, file_buffer, entry.size);
            free(file_buffer);
        }
    }

    // Закрываем файловые дескрипторы
    close(temp_fd);
    close(arch_fd);

    // Если файл был найден и извлечен, то заменяем старый архив на новый
    if (found) {
        rename("temp_archive", arch_name);
        printf("Файл %s извлечен и удален из архива.\n", file_name);
    } else {
        // Если файл не найден, удаляем временный архив
        remove("temp_archive");
        printf("Файл %s не найден в архиве.\n", file_name);
    }
}

// Главная функция
int main(int argc, char *argv[]) {
    if (argc < 3) {
        usage(); // Если аргументов недостаточно, показываем справку
        return EXIT_FAILURE;
    }

    const char *arch_name = argv[1]; // Имя архива
    int arch_fd = open(arch_name, O_RDWR | O_APPEND | O_CREAT, 0666); // Открываем архив
    if (arch_fd == -1) {
        perror("Ошибка создания/открытия архива");
        return EXIT_FAILURE;
    }

    // Обработка различных команд
    if (strcmp(argv[2], "-i") == 0 || strcmp(argv[2], "--input") == 0) {
        for (int i = 3; i < argc; i++) {
            add_file_to_archive(arch_fd, argv[i]); // Добавляем файлы в архив
        }
    } else if (strcmp(argv[2], "-s") == 0 || strcmp(argv[2], "--stat") == 0) {
        show_archive_stat(arch_fd); // Показываем состояние архива

    } else if (strcmp(argv[2], "-e") == 0 || strcmp(argv[2], "--extract") == 0) {
        if (argc < 4) {
            usage(); // Если не указан файл для извлечения, показываем справку
            close(arch_fd);
            return EXIT_FAILURE;
        }

        extract_file_from_archive(arch_name, argv[3]); // Извлекаем файл из архива
        return EXIT_SUCCESS; // Завершаем программу
    } else if (strcmp(argv[2], "-h") == 0 || strcmp(argv[2], "--help") == 0) {
        usage(); // Если запрашивается справка, выводим ее
        close(arch_fd);
        return EXIT_SUCCESS;
    } else {
        usage(); // Если команда не распознана, показываем справку
        close(arch_fd);
        return EXIT_FAILURE;
    }

    close(arch_fd); // Закрываем файловый дескриптор
    return EXIT_SUCCESS;
}
