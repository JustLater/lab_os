#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024 // Максимальная длина строки

int main(int argc, char *argv[]) {
    // Флаги для управления поведением утилиты
    int print_line_numbers = 0; // Печатать номера строк
    int print_blank_lines = 0; // Печатать пустые строки
    int print_dollar_sign = 0; // Печатать символ "$" в конце каждой строки
    FILE *file; // Указатель на открытый файл
    char line[MAX_LINE_LENGTH]; // Буфер для хранения текущей строки
    int line_number = 0; // Номер текущей строки

    // Разбор аргументов командной строки
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            print_line_numbers = 1; // Включить нумерацию строк
        } else if (strcmp(argv[i], "-b") == 0) {
            print_blank_lines = 1; // Включить печать пустых строк
            print_line_numbers = 1; // Включить нумерацию строк
        } else if (strcmp(argv[i], "-E") == 0) {
            print_dollar_sign = 1; // Включить печать символа "$" в конце строк
        } else {
            // Открытие файла для чтения
            file = fopen(argv[i], "r");
            if (file == NULL) {
                fprintf(stderr, "Error: Could not open file '%s'\n", argv[i]);
                return 1;
            }

            // Печать содержимого файла
            while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
                // Если включена печать пустых строк или строка не пустая
                if (print_blank_lines || (strlen(line) > 1 || line[0] != '\n')) {
                    // Если включена нумерация строк, печатаем номер
                    if (print_line_numbers) {
                        printf("%6d\t", line_number + 1);
                    }
                    // Печатаем текущую строку
                    printf("%s", line);
                    // Если включена печать символа "$" и строка не заканчивается на "\n",
                    // печатаем символ "$" в конце строки
                    if (print_dollar_sign && line[strlen(line) - 2] != '\n') {
                        printf("$");
                    }
                    line_number++;
                }
            }

            fclose(file);
        }
    }

    return 0;
}


