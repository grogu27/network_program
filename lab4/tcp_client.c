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
    const char *server_ip = argv[1];
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Ошибка создания сокета");
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    
    servAddr.sin_port = htons(atoi(argv[2]));
   
    if (inet_pton(AF_INET, server_ip, &servAddr.sin_addr) <= 0) {
      perror("Неверный адрес сервера");
      close(sock);
      exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("Ошибка подключения");
        exit(1);
    }

    printf("КЛИЕНТ: Подключение успешно. Отправляем число i с задержкой i сек...\n");
    char buffer[32];
    int num2 = num;
    
    for (;;) {
        sprintf(buffer, "%d", num);
        if (send(sock, buffer, sizeof(buffer), 0) < 0) {
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
