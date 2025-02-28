#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFLEN 81

int main() {
    int sockMain, msgLength;
    struct sockaddr_in servAddr, clientAddr;
    char buf[BUFLEN];

    
    if ((sockMain = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Ошибка при создании сокета UDP");
        exit(1);
    }

    
   // memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = 0; 

    if (bind(sockMain, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("Ошибка привязки сервера");
        close(sockMain);
        exit(1);
    }

    socklen_t length = sizeof(servAddr);
    if (getsockname(sockMain, (struct sockaddr*)&servAddr, &length) < 0) {
        perror("Ошибка при получении номера порта");
        close(sockMain);
        exit(1);
    }

    printf("SERVER: Номер выбранного порта: %d\n", ntohs(servAddr.sin_port));

    while (1) {
        socklen_t clientLen = sizeof(clientAddr);
        //memset(buf, 0, BUFLEN);

        msgLength = recvfrom(sockMain, buf, BUFLEN, 0, 
                             (struct sockaddr*)&clientAddr, &clientLen);
        if (msgLength < 0) {
            perror("Ошибка получения данных от клиента");
            continue;
        }

        buf[msgLength] = '\0';  
        int receivedNumber = atoi(buf);

        printf("SERVER: IP клиента: %s, порт: %d\n", 
               inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        printf("SERVER: Получено число: %d\n", receivedNumber);

        int newValue = receivedNumber + 1;
        sprintf(buf, "%d", newValue);

        sendto(sockMain, buf, strlen(buf), 0, 
               (struct sockaddr*)&clientAddr, clientLen);
        
        printf("SERVER: Отправлено число: %d\n\n", newValue);
    }

    close(sockMain);
    return 0;
}
