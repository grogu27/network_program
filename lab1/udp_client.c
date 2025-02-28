#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFLEN 81

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Использование: %s <IP сервера> <порт> <число>\n", argv[0]);
        exit(1);
    }

    int sock;
    struct sockaddr_in servAddr;
    char buf[BUFLEN];

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Ошибка при создании сокета");
        exit(1);
    }

    //memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(atoi(argv[2])); 
    if (inet_pton(AF_INET, argv[1], &servAddr.sin_addr) <= 0) {
        perror("Ошибка в inet_pton (неправильный IP)");
        close(sock);
        exit(1);
    }

    int i = atoi(argv[3]);

    //memset(buf, 0, BUFLEN);
    sprintf(buf, "%d", i);
    while (1)
    {
        
        if (sendto(sock, buf, strlen(buf), 0, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
            perror("Ошибка отправки данных серверу");
            close(sock);
            exit(1);
        }
        socklen_t servLen = sizeof(servAddr);
        int recvLen = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr*)&servAddr, &servLen);
        if (recvLen < 0) {
            perror("Ошибка получения ответа от сервера");
            close(sock);
            exit(1);
        }
        buf[recvLen] = '\0';
        printf("CLIENT: Ответ сервера: %s\n", buf);
        sleep(i);
    }
    close(sock);
    return 0;
}
