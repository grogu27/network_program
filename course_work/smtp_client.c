#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 1025
#define BUFSIZE 1024

FILE *log_file;

void log_print(const char *label, const char *text) {
    //printf("%s: %s", label, text);
    if (log_file) {
        fprintf(log_file, "%s: %s", label, text);
    }
}

void recv_response(int sock) {
    char buf[BUFSIZE] = {0};
    int len = recv(sock, buf, BUFSIZE - 1, 0);
    if (len > 0) {
        buf[len] = '\0';
        log_print("\nSERVER", buf);
    } else {
        log_print("\nSERVER", "(no response or error)\n");
    }
}

void send_cmd(int sock, const char *cmd, const char *explain) {
    send(sock, cmd, strlen(cmd), 0);
    char labeled[BUFSIZE];
    snprintf(labeled, BUFSIZE, "%s (Explanation: %s)", cmd, explain);
    log_print("CLIENT", labeled);
    recv_response(sock);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <smtp_server>\n", argv[0]);
        return 1;
    }

    const char *server_addr = argv[1];
    log_file = fopen("smtp_log.txt", "w");
    if (!log_file) {
        perror("fopen");
        return 1;
    }

    struct hostent *host = gethostbyname(server_addr);
    if (!host) {
        perror("gethostbyname");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    memcpy(&server.sin_addr, host->h_addr_list[0], host->h_length);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        return 1;
    }

    recv_response(sock);

    send_cmd(sock, "HELO client.local\r\n", "Представление клиента");
    send_cmd(sock, "MAIL FROM:<iv222@example.com>\r\n", "Указание отправителя");
    send_cmd(sock, "RCPT TO:<recipient@example.com>\r\n", "Указание получателя");
    send_cmd(sock, "DATA\r\n", "Начало передачи тела письма");

    // const char *body =
    //     "Subject: Hello from C client\r\n"
    //     "From: iv222@example.com\r\n"
    //     "To: recipient@example.com\r\n"
    //     "\r\n"
    //     "Hello world,\r\n"
    //     "This is a test message sent from an SMTP client.\r\n"
    //     ".\r\n";

   // printf("Открываю message.txt...\n");
    FILE *msg = fopen("message.txt", "r");
    if (!msg) {
        perror("fopen message");
        return 1;
    }

   //printf("Файл открыт успешно.\n");

    char body[BUFSIZE * 4] = {0}; 
    snprintf(body, sizeof(body),
        "Subject: Hello from C client\r\n"
        "From: iv222s14@example.com\r\n"
        "To: recipient@example.com\r\n"
        "\r\n");

    char line[BUFSIZE];
    while (fgets(line, sizeof(line), msg)) 
        strncat(body, line, sizeof(body) - strlen(body) - 1);
    
    if (body[strlen(body) - 1] != '\n') 
        strncat(body, "\r\n", sizeof(body) - strlen(body) - 1);
    
    strncat(body, ".\r\n", sizeof(body) - strlen(body) - 1);
    fclose(msg);

    send_cmd(sock, body, "Отправка тела письма, заканчивающегося точкой");

    send_cmd(sock, "QUIT\r\n", "Завершение SMTP-сессии");

    close(sock);
    fclose(log_file);
    printf("Письмо отправлено\n");
    return 0;
}
