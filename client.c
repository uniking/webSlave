#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <json-c/json.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// 从文件中读取JSON内容
char* read_json_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *json_content = malloc(length + 1);
    if (!json_content) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    fread(json_content, 1, length, file);
    json_content[length] = '\0';
    fclose(file);

    return json_content;
}

// 持续读取服务端数据直到连接关闭
void receive_from_server(int sock) {
    char buffer[BUFFER_SIZE] = {0};
    int valread;

    //printf("Response from server:\n");

    // 循环读取服务端返回的数据，直到 socket 关闭
    while ((valread = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[valread] = '\0';
        printf("%s", buffer); // 持续打印服务端传来的数据
    }

    // if (valread == 0) {
    //     printf("\nConnection closed by server\n");
    // } else {
    //     perror("recv failed");
    // }
}

// 客户端程序
int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <json_file>\n", argv[0]);
        return 1;
    }

    int sock = 0;
    struct sockaddr_in serv_addr;

    // 读取JSON文件内容
    char *json_command = read_json_from_file(argv[1]);

    // 创建socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 将服务端地址转换为二进制形式
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // 连接服务端
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // 发送JSON命令
    send(sock, json_command, strlen(json_command), 0);
    //printf("JSON command sent: %s\n", json_command);

    // 如果是同步命令，接收服务端返回的数据
    struct json_object *parsed_json;
    struct json_object *sync;

    parsed_json = json_tokener_parse(json_command);
    json_object_object_get_ex(parsed_json, "sync", &sync);
    int is_sync = json_object_get_boolean(sync);

    if (is_sync) {
        // 持续读取直到服务端关闭连接
        receive_from_server(sock);
    }

    free(json_command);
    close(sock);
    return 0;
}
