#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *message = "{\"cmd\":\"mount\", \"uuid\":\"614eacee-debc-4f0c-bf33-22955999466d\", \"path\":\"/home/arch/samb\"}";
    char buffer[BUFFER_SIZE] = {0};

    // 创建socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 将地址转换为二进制
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // 连接到服务器
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // 发送JSON消息
    send(sock, message, strlen(message), 0);
    printf("Message sent\n");

    // 读取服务器返回的数据
    int valread = read(sock, buffer, BUFFER_SIZE);
    printf("Server response: %s\n", buffer);

    // 关闭连接
    close(sock);

    return 0;
}
