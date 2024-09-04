#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define WEB_ROOT "./www"

void serve_file(int client_fd, const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        const char *not_found_response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found";
        write(client_fd, not_found_response, strlen(not_found_response));
        return;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    char *content = malloc(filesize + 1);
    fread(content, 1, filesize, file);
    fclose(file);

    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: text/html\r\n\r\n", filesize);
    write(client_fd, header, strlen(header));
    write(client_fd, content, filesize);
    
    free(content);
}

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read < 0) {
        perror("Read from client failed");
        return;
    }

    buffer[bytes_read] = '\0';
    printf("Received request:\n%s\n", buffer);
    if (strncmp(buffer, "GET / ", 6) == 0) {
        serve_file(client_fd, WEB_ROOT "/index.html");
    } else if (strncmp(buffer, "GET ", 4) == 0) {
        char filepath[256];
        sscanf(buffer, "GET /%s ", filepath);
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", WEB_ROOT, filepath);
        serve_file(client_fd, fullpath);
    } else {
        const char *method_not_allowed_response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\n\r\n405 Method Not Allowed";
        write(client_fd, method_not_allowed_response, strlen(method_not_allowed_response));
    }
}
