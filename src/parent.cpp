#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <cstring>
#include <unistd.h>
#include <string>


int main(){
    int memoryd;
    memoryd = open(
        "memory.txt",      // Имя файла
        O_RDWR | O_CREAT | O_TRUNC,  // Флаги открытия файла (чтение и запись, создание файла, обрезать его до нулевой длины)
        0666               // Режим доступа к файлу (права доступа)
    );

    if (memoryd == -1) {
        perror("Error opening file");
        return -1;
    }


    ftruncate(memoryd, 1024);//обрезание memory.txt до 1024 байт

    char* buffer = (char*)mmap(
        NULL,        // Адрес начала отображения (NULL означает, что ядро само выбирает адрес)
        1024,        // Размер отображаемой области в байтах
        PROT_READ | PROT_WRITE,  // Разрешения на чтение и запись
        MAP_SHARED,  // Общий режим отображения (изменения видны другим процессам)
        memoryd,     // Файловый дескриптор, с которым связано отображение
        0            // Смещение в файле (0 означает отображение с начала файла)
    );

    close(memoryd);

    sem_t* sem = sem_open("mmap_sem", O_CREAT, 0777, 0);  // Open semaphore
    //"mmap_sem": Это строка, представляющая имя семафора. В данном случае, семафор называется "mmap_sem".
    //O_CREAT: Флаг, указывающий, что семафор должен быть создан, если его нет. Если семафор существует, он будет открыт.
    //0777: Это восьмеричное число, представляющее режим доступа к семафору. Здесь 0777 означает, что семафор доступен для чтения, записи и выполнения для всех пользователей.
    //0: Это начальное значение семафора. В данном случае, семафор инициализируется значением 0.

    if (sem == SEM_FAILED) 
    {
        perror("Could not open semaphore");
        return -1;
    }

    int id1 = fork();
    if (id1 == -1)
        return 2;
    else if(id1 > 0){
        int id2 = fork();
        if (id2 == -1)
            return 2;
        else if(id2 > 0){
            char c;
            c = getchar();
            int i = 0;

            while (c != EOF) 
            {
                buffer[i++] = c;
                c = getchar();
            }
            buffer[i] = c;

            sem_post(sem);            

            int status1, status2;
            
            waitpid(id1, &status1, 0);
            waitpid(id2, &status2, 0);
            sem_wait(sem);
            sem_close(sem);
            munmap(buffer, 1024);
            if (status1 != 0)
                perror("error");
            return status1;
        }
        else{
            execl("./child", "./child", "mmap_sem", NULL);
            perror("execl");
            exit(EXIT_FAILURE);
        }
    }
    else{
        execl("./child", "./child", "mmap_sem", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    }

}