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

// void sigint_handler(int signum) {
//     printf("\nСЕРВЕР: Получен сигнал SIGINT, завершаем работу...\n");
//     close(sockMain);
//     pthread_mutex_destroy(&lock);
//     exit(0);
// }

void* handle_client(void *arg) {
    int client_sock;
    struct sockaddr_in clientAddr;
    char buf[BUFLEN];
    FILE *fp;

    int *data = (int *)arg;
    client_sock = data[0];
    memcpy(&clientAddr, data + 1, sizeof(struct sockaddr_in));
    free(data);

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
    //signal(SIGINT, sigint_handler);
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
        
        int *arg = malloc(sizeof(int) + sizeof(struct sockaddr_in));
        if (!arg) {
            perror("malloc");
            close(sockClient);
            continue;
        }

        arg[0] = sockClient;
        memcpy(arg + 1, &clientAddr, sizeof(struct sockaddr_in));

        if (pthread_create(&tid, &ta, handle_client, arg) != 0) {
            perror("Поток не создался");
            close(sockClient);
            free(arg);
        }
    }

    close(sockMain);
    pthread_mutex_destroy(&lock);
    return 0;
}
