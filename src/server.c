#include "server.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CONNECTIONS 10

void *client_thread(void *arg) {
    int client_fd = *(int *)arg;
    free(arg); 
    handle_client(client_fd);
    close(client_fd);
    return NULL;
}

int start_server(int port) {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, MAX_CONNECTIONS) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("Server started on port %d\n", port);

    while (1) {
        int *client_fd = malloc(sizeof(int));
        if (client_fd == NULL) {
            perror("Failed to allocate memory for client_fd");
            continue;
        }

        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (*client_fd < 0) {
            perror("Accept failed");
            free(client_fd);
            continue;
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, client_fd) != 0) {
            perror("Failed to create thread");
            close(*client_fd);
            free(client_fd);
            continue;
        }

        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
