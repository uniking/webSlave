#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <json-c/json.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// 定义链表节点结构体
typedef struct AsyncCommand {
    char command[BUFFER_SIZE];
    struct AsyncCommand *next;
} AsyncCommand;

// 定义链表和互斥锁
AsyncCommand *head = NULL;
pthread_mutex_t lock;

// 将异步指令添加到链表
void add_async_command(const char *command) {
    pthread_mutex_lock(&lock);
    AsyncCommand *new_command = (AsyncCommand *)malloc(sizeof(AsyncCommand));
    strncpy(new_command->command, command, BUFFER_SIZE);
    new_command->next = head;
    head = new_command;
    pthread_mutex_unlock(&lock);
}

// 从链表中移除异步指令
void remove_async_command(const char *command) {
    pthread_mutex_lock(&lock);
    AsyncCommand *current = head;
    AsyncCommand *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->command, command) == 0) {
            if (prev == NULL) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(&lock);
}

// 处理命令执行和输出的函数
void execute_command(const char *command, int new_socket, int is_sync) {
    FILE *fp;
    char output[BUFFER_SIZE] = {0};

    // 执行命令
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run command");
        if (is_sync) {
            const char *error_message = "Command execution failed\n";
            send(new_socket, error_message, strlen(error_message), 0);
        }
        return;
    }

    // 获取命令输出
    if (is_sync) {
        while (fgets(output, sizeof(output), fp) != NULL) {
            send(new_socket, output, strlen(output), 0);
        }
    }
    pclose(fp);
}

// 处理 "currentAsyn" 命令
void handle_current_async(int new_socket) {
    pthread_mutex_lock(&lock);
    AsyncCommand *current = head;
    char response[BUFFER_SIZE * 10] = {0};

    while (current != NULL) {
        strncat(response, current->command, BUFFER_SIZE);
        strncat(response, "\n", 2);
        current = current->next;
    }

    send(new_socket, response, strlen(response), 0);
    pthread_mutex_unlock(&lock);
}

void *handle_client(void *socket_desc) {
    int new_socket = *(int *)socket_desc;
    char buffer[BUFFER_SIZE] = {0};
    read(new_socket, buffer, BUFFER_SIZE);

    // 解析JSON数据
    struct json_object *parsed_json;
    struct json_object *cmd;
    struct json_object *sync;
    struct json_object *uuid;
    struct json_object *path;
    struct json_object *times;
    struct json_object *operation;
    struct json_object *target;

    parsed_json = json_tokener_parse(buffer);
    json_object_object_get_ex(parsed_json, "cmd", &cmd);
    json_object_object_get_ex(parsed_json, "sync", &sync);

    const char *command_str = json_object_get_string(cmd);
    int is_sync = json_object_get_boolean(sync);

    if (strcmp(command_str, "sleep") == 0) {
        // 处理 "sleep" 指令
        json_object_object_get_ex(parsed_json, "times", &times);
        char command[BUFFER_SIZE];
        snprintf(command, sizeof(command), "sleep %s", json_object_get_string(times));

        printf("Executing sleep command: %s\n", command);

        if (is_sync) {
            execute_command(command, new_socket, 1);
        } else {
            // 异步执行
            add_async_command(buffer);
            close(new_socket);
            execute_command(command, -1, 0);
            remove_async_command(buffer);
        }

    } else if (strcmp(command_str, "mount") == 0) {
        // 处理 "mount" 指令
        json_object_object_get_ex(parsed_json, "uuid", &uuid);
        json_object_object_get_ex(parsed_json, "path", &path);

        char command[BUFFER_SIZE];
        snprintf(command, sizeof(command), "%s UUID=%s %s", 
                 json_object_get_string(cmd), 
                 json_object_get_string(uuid), 
                 json_object_get_string(path));

        printf("Executing mount command: %s\n", command);

        if (is_sync) {
            execute_command(command, new_socket, 1);
        } else {
            // 异步执行
            add_async_command(buffer);
            close(new_socket);
            execute_command(command, -1, 0);
            remove_async_command(buffer);
        }
    } else if (strcmp(command_str, "docker") == 0) {
        // 处理 "docker" 指令
        json_object_object_get_ex(parsed_json, "operation", &operation);
        json_object_object_get_ex(parsed_json, "target", &target);

        char command[BUFFER_SIZE];
        snprintf(command, sizeof(command), "%s %s %s", 
                 json_object_get_string(cmd), 
                 json_object_get_string(operation),
                 json_object_get_string(target));

        printf("Executing docker command: %s\n", command);

        if (is_sync) {
            execute_command(command, new_socket, 1);
        } else {
            // 异步执行
            add_async_command(buffer);
            close(new_socket);
            execute_command(command, -1, 0);
            remove_async_command(buffer);
        }
    } else if (strcmp(command_str, "umount") == 0) {
        // 处理 "umount" 指令
        json_object_object_get_ex(parsed_json, "path", &path);

        char command[BUFFER_SIZE];
        snprintf(command, sizeof(command), "%s %s", 
                 json_object_get_string(cmd), 
                 json_object_get_string(path));

        printf("Executing umount command: %s\n", command);

        if (is_sync) {
            execute_command(command, new_socket, 1);
        } else {
            // 异步执行
            add_async_command(buffer);
            close(new_socket);
            execute_command(command, -1, 0);
            remove_async_command(buffer);
        }
    } else if (strcmp(command_str, "shutdown") == 0) {
        // 处理 "shutdown" 指令
        char command[BUFFER_SIZE];
        snprintf(command, sizeof(command), "shutdown -h now");

        printf("Executing shutdown command: %s\n", command);

        if (is_sync) {
            execute_command(command, new_socket, 1);
        } else {
            // 异步执行
            add_async_command(buffer);
            close(new_socket);
            execute_command(command, -1, 0);
            remove_async_command(buffer);
        }
    } else if (strcmp(command_str, "reboot") == 0) {
        // 处理 "reboot" 指令
        char command[BUFFER_SIZE];
        snprintf(command, sizeof(command), "reboot");

        printf("Executing reboot command: %s\n", command);

        if (is_sync) {
            execute_command(command, new_socket, 1);
        } else {
            // 异步执行
            add_async_command(buffer);
            close(new_socket);
            execute_command(command, -1, 0);
            remove_async_command(buffer);
        }
    } else if (strcmp(command_str, "currentAsyn") == 0) {
        // 处理 "currentAsyn" 指令
        handle_current_async(new_socket);
        close(new_socket);
    }

    free(socket_desc);
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // 初始化互斥锁
    if (pthread_mutex_init(&lock, NULL) != 0) {
        perror("Mutex init failed");
        return 1;
    }

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

        // 创建线程处理客户端请求
        pthread_t thread_id;
        int *new_sock = malloc(sizeof(int));
        *new_sock = new_socket;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }

        // 分离线程，以便在完成后自行清理资源
        pthread_detach(thread_id);
    }

    // 销毁互斥锁
    pthread_mutex_destroy(&lock);

    return 0;
}
