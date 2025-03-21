#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define BUFLEN 81
#define BACKLOG 5

void reaper(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0); // Чистка завершившихся дочерних процессов
}

void handle_client(int client_sock, struct sockaddr_in clientAddr) {
    char buf[BUFLEN];
    int msgLength;

    while(1) {
        memset(buf, 0, BUFLEN);
        if ((msgLength = recv(client_sock, buf, BUFLEN, 0)) < 0) {
            perror("Ошибка при получении данных от клиента");
            close(client_sock);
            exit(1);
        }

        if (msgLength == 0) {
            printf("СЕРВЕР: Соединение закрыто клиентом (Socket %d)\n", client_sock);
            break;
        }
        //printf("СЕРВЕР: Получено сообщение от клиента %s:%d (Socket %d): Сообщение: %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), client_sock, buf);
        printf("СЕРВЕР: Получено сообщение от клиента %s:%d : Сообщение: %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buf);
    }
    close(client_sock);
    exit(0);
}

int main() {
    int sockMain, sockClient;
    struct sockaddr_in servAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    signal(SIGCHLD, reaper); 

    if ((sockMain = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Ошибка создания сокета");
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = 0; 

    if (bind(sockMain, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("Ошибка связывания сокета");
        close(sockMain);
        exit(1);
    }

    socklen_t length = sizeof(servAddr);
    if (getsockname(sockMain, (struct sockaddr*)&servAddr, &length) < 0) {
        perror("Ошибка getsockname");
        close(sockMain);
        exit(1);
    }

    printf("СЕРВЕР: Запущен на порту %d\n", ntohs(servAddr.sin_port));

    if (listen(sockMain, BACKLOG) < 0) {
        perror("Ошибка при вызове listen");
        close(sockMain);
        exit(1);
    }

    while (1) {
        if ((sockClient = accept(sockMain, (struct sockaddr*)&clientAddr, &clientLen)) < 0) {
            if (errno == EINTR) continue; // Игнорируем прерывания
            perror("Ошибка accept");
            close(sockMain);
            exit(1);
        }

        printf("СЕРВЕР: Подключен клиент %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        pid_t pid = fork();
        if (pid == 0) { // Дочерний процесс
            printf("СЕРВЕР: Создан дочерний процесс для клиента\n");
            close(sockMain); // Дочернему процессу не нужен основной сокет
            handle_client(sockClient, clientAddr);
        } else if (pid > 0) {
            close(sockClient); // Родителю не нужен клиентский сокет
        } else {
            perror("Ошибка fork");
            close(sockClient);
            close(sockMain);
            exit(1);
        }
    }

    close(sockMain); // Этот код не выполнится, так как цикл бесконечный
    return 0;
}
