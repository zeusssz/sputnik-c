#include "server.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CONNECTIONS 50
#define MAX_BACKLOG 10

typedef struct {
    int client_fd;
    char client_ip[INET6_ADDRSTRLEN];
} client_info_t;

volatile int running = 1;
void handle_signal(int sig) {
    printf("Received signal %d. Shutting down...\n", sig);
    running = 0;
}

void *client_thread(void *arg) {
    client_info_t *info = (client_info_t *)arg;
    handle_client(info->client_fd, info->client_ip);
    close(info->client_fd);
    free(info);
    return NULL;
}

int start_server(int port) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(server_fd);
        return 1;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);t
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }
    if (listen(server_fd, MAX_BACKLOG) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }
    
    printf("Server started on port %d\n", port);
    printf("Serving files from ./www directory\n");
    printf("Press Ctrl+C to stop the server\n");
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int select_result = select(server_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (select_result < 0) {
            if (errno == EINTR) continue;
            perror("Select error");
            break;
        } else if (select_result == 0) {
            continue;
        }
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            perror("Accept failed");
            continue;
        }
        client_info_t *client_info = malloc(sizeof(client_info_t));
        if (!client_info) {
            perror("Failed to allocate memory for client info");
            close(client_fd);
            continue;
        }
        
        client_info->client_fd = client_fd;
        inet_ntop(AF_INET, &client_addr.sin_addr, client_info->client_ip, INET6_ADDRSTRLEN);
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, client_info) != 0) {
            perror("Failed to create thread");
            close(client_fd);
            free(client_info);
            continue;
        }
        pthread_detach(tid);
    }
    printf("\nShutting down server...\n");
    close(server_fd);
    return 0;
}