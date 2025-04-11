#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define BUFLEN 81
#define BACKLOG 5

pthread_mutex_t lock;
//int sockMain;

typedef struct {
    int sock;
    struct sockaddr_in addr;
} client_info_t;

// void sigint_handler(int signum) {
//     printf("\nСЕРВЕР: Получен сигнал SIGINT, завершаем работу...\n");
//     close(sockMain);
//     pthread_mutex_destroy(&lock);
//     exit(0);
// }

void* handle_client(void *arg) {
    //signal(SIGINT, sigint_handler);

    client_info_t *info = (client_info_t *)arg;
    int client_sock = info->sock;
    struct sockaddr_in clientAddr = info->addr;
    free(info);

    char buf[BUFLEN];
    FILE *fp;

    while (1) {
        memset(buf, 0, BUFLEN);
        int msgLength = recv(client_sock, buf, BUFLEN, 0);
        if (msgLength < 0) {
            perror("recv");
            break;
        } else if (msgLength == 0) {
            printf("СЕРВЕР: Клиент %s:%d отключился\n",
                   inet_ntoa(clientAddr.sin_addr),
                   ntohs(clientAddr.sin_port));
            break;
        }

        printf("СЕРВЕР: Получено сообщение от клиента %s:%d : %s\n",
               inet_ntoa(clientAddr.sin_addr),
               ntohs(clientAddr.sin_port),
               buf);

        pthread_mutex_lock(&lock);
        fp = fopen("log.txt", "a");
        if (fp) {
            fprintf(fp, "От %s:%d : %s\n",
                    inet_ntoa(clientAddr.sin_addr),
                    ntohs(clientAddr.sin_port),
                    buf);
            fclose(fp);
        } else {
            perror("Не открылся файл");
        }
        pthread_mutex_unlock(&lock);
    }

    close(client_sock);
    pthread_exit(NULL);
}

int main() {
    int sockMain, sockClient;
    struct sockaddr_in servAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    pthread_t tid;
    pthread_attr_t ta;

    pthread_mutex_init(&lock, NULL);
    pthread_attr_init(&ta);
    pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);

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
            perror("Ошибка accept");
            continue;
        }

        printf("СЕРВЕР: Подключен клиент %s:%d\n",
               inet_ntoa(clientAddr.sin_addr),
               ntohs(clientAddr.sin_port));

        client_info_t *info = malloc(sizeof(client_info_t));
        if (!info) {
            perror("malloc");
            close(sockClient);
            continue;
        }

        info->sock = sockClient;
        info->addr = clientAddr;

        if (pthread_create(&tid, &ta, handle_client, info) != 0) {
            perror("Поток не создался");
            close(sockClient);
            free(info);
        }
    }

    close(sockMain);
    pthread_mutex_destroy(&lock);
    return 0;
}
