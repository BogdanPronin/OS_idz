#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const int buffer_size = 5000;

void reverseString(char *str) {
    if (str == NULL) { // проверяем, что указатель на строку не пустой
        return;
    }
    char *start = str;
    char *end = start + strlen(str) - 1;
    while (start < end) {
        // меняем символы, на которые указывают указатели
        char temp = *start;
        *start = *end;
        *end = temp;
        // перемещаем указатели к центру строки
        start++;
        end--;
    }
}


int main(int argc, char *argv[]) {
    char buffer[buffer_size];
    if (argc != 3) {
        printf("Usage: input_file output_file\n");
        return 0;
    }
    int feed_1_2[2], feed_2_3[2];
    pipe(feed_1_2); // Канал для передачи первого процесса второму
    pipe(feed_2_3); // Канал для передачи второго процесса третьему
    int process;
    // Cоздаем 2 процесса
    process = fork();
    if (process == 0) {
        // Второй процесс
        // Разделяем второй процесс на второй и третий
        int process_2_3 = fork();
        if (process_2_3 == 0) {
            // Третий процесс должен прочитать результат выполнения функции, который был передан во второй канал
            // от второго процесса.
            // Затем этот результат записываем в выходной файл.
            close(feed_2_3[1]);
            // Считываем второй канал
            read(feed_2_3[0], buffer, buffer_size);
            close(feed_2_3[0]);
            int file_out = open(argv[2], O_WRONLY | O_CREAT, 0666);
            // Записываем в файл
            write(file_out, buffer, strlen(buffer));
            close(file_out);
        } else {
            // Второй процесс должен прочитать информацию из первого канала, которая содержит данные из входного файла.
            // Далее вызываем функцию для получения результата.
            // Полученный результат направляем во второй канал для передачи в третий процесс.
            close(feed_1_2[1]);
            read(feed_1_2[0], buffer, buffer_size);
            reverseString(buffer);
            close(feed_1_2[0]);
            write(feed_2_3[1], buffer, buffer_size);
            close(feed_2_3[1]);
        }
    } else {
        // Первый процесс считывает данные из файла и направляет дальше
        close(feed_1_2[0]);
        int file_in = open(argv[1], O_RDONLY, 0666);
        read(file_in, buffer, buffer_size);
        close(file_in);
        // Записываем в первый канал
        write(feed_1_2[1], buffer, buffer_size);
        close(feed_1_2[1]);
    }

    return 0;
}