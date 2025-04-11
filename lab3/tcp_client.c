#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr,"Использование: %s <IP> <порт> <число i>\n", argv[0]);
        exit(1);
    }

    int sock;
    struct sockaddr_in servAddr;
    struct hostent *server;
    int num = atoi(argv[3]);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Ошибка создания сокета");
        exit(1);
    }

    if ((server = gethostbyname(argv[1])) == NULL) {
        perror("Ошибка gethostbyname\n");
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    
    char **addr_list = server->h_addr_list;
    servAddr.sin_addr.s_addr = *(in_addr_t *)addr_list[0];
    
    servAddr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("Ошибка подключения");
        exit(1);
    }

    printf("КЛИЕНТ: Подключение успешно. Отправляем число i с задержкой i сек...\n");
    char buffer[32];
    int num2 = num;
    
    for (;;) {
        sprintf(buffer, "%d", num);
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("Ошибка отправки");
            exit(1);
        }
        printf("КЛИЕНТ: Отправлено сообщение: %s\n", buffer);
        sleep(num);
        
    }

    printf("КЛИЕНТ: Отправка завершена, закрываем соединение.\n");
    close(sock);
    return 0;
}
