#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <json-c/json.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int new_socket) {
    char buffer[BUFFER_SIZE] = {0};
    read(new_socket, buffer, BUFFER_SIZE);

    // 解析JSON数据
    struct json_object *parsed_json;
    struct json_object *cmd;
    struct json_object *uuid;
    struct json_object *path;

    parsed_json = json_tokener_parse(buffer);
    json_object_object_get_ex(parsed_json, "cmd", &cmd);
    json_object_object_get_ex(parsed_json, "uuid", &uuid);
    json_object_object_get_ex(parsed_json, "path", &path);

    // 构造并执行命令
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "%s UUID=%s %s", 
             json_object_get_string(cmd), 
             json_object_get_string(uuid), 
             json_object_get_string(path));

    printf("Executing command: %s\n", command);

    FILE *fp;
    char output[BUFFER_SIZE] = {0};
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run command");
        exit(1);
    }

    // 获取命令输出
    fread(output, sizeof(char), BUFFER_SIZE, fp);
    pclose(fp);

    // 返回命令输出到客户端
    send(new_socket, output, strlen(output), 0);

    // 关闭连接
    close(new_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // 创建socket文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 绑定端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听端口
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Waiting for connection...\n");

        // 接受客户端连接
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                           (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Connection established\n");
        handle_client(new_socket);
    }

    return 0;
}
