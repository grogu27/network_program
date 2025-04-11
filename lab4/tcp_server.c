#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUFLEN 81
#define BACKLOG 5

int main() {
    int sockMain, sockClient, nfds;
    struct sockaddr_in servAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    fd_set active_set, read_set;

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

    nfds = sockMain;  
    FD_ZERO(&active_set);
    FD_SET(sockMain, &active_set);

    while (1) {
        memcpy(&read_set, &active_set, sizeof(read_set));
        if (select(nfds + 1, &read_set, NULL, NULL, NULL) < 0) {
            perror("Ошибка select");
            continue;
        }

        if (FD_ISSET(sockMain, &read_set)) {
            sockClient = accept(sockMain, (struct sockaddr*)&clientAddr, &clientLen);
            if (sockClient < 0) {
                perror("Ошибка accept");
                continue;
            }

            FD_SET(sockClient, &active_set);
            nfds = (sockClient > nfds) ? sockClient : nfds;  
            printf("СЕРВЕР: Подключен клиент %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        }

        for (int fd = 0; fd <= nfds; fd++) {
            if (fd != sockMain && FD_ISSET(fd, &read_set)) {
                char buf[BUFLEN] = {0};
                int msgLength = recv(fd, buf, BUFLEN, 0);

                if (msgLength <= 0) {
                    if (msgLength == 0) {
                        printf("СЕРВЕР: Клиент отключился (fd=%d)\n", fd);
                    } else {
                        perror("Ошибка recv");
                    }
                    close(fd);
                    FD_CLR(fd, &active_set);
                } else {
                    printf("СЕРВЕР: Получено сообщение от клиента %s:%d (fd=%d): %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), fd, buf);

                    FILE *fp = fopen("log.txt", "a");
                    if (fp) {
                        fprintf(fp, "От fd %d: %s\n", fd, buf);
                        fclose(fp);
                    } else {
                        perror("Не открылся файл");
                    }
                }
            }
        }
    }

    close(sockMain);
    return 0;
}
