#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096
#define WEB_ROOT "./www"
#define LOG_DIR "./logs"
const char* get_content_type(const char *filepath) {
    const char *ext = strrchr(filepath, '.');
    
    if (!ext) return "application/octet-stream";
    
    if (strcasecmp(ext, ".html") == 0 || strcasecmp(ext, ".htm") == 0) return "text/html";
    if (strcasecmp(ext, ".css") == 0) return "text/css";
    if (strcasecmp(ext, ".js") == 0) return "application/javascript";
    if (strcasecmp(ext, ".json") == 0) return "application/json";
    if (strcasecmp(ext, ".png") == 0) return "image/png";
    if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcasecmp(ext, ".gif") == 0) return "image/gif";
    if (strcasecmp(ext, ".svg") == 0) return "image/svg+xml";
    if (strcasecmp(ext, ".ico") == 0) return "image/x-icon";
    if (strcasecmp(ext, ".txt") == 0) return "text/plain";
    if (strcasecmp(ext, ".pdf") == 0) return "application/pdf";
    
    return "application/octet-stream";
}
int is_path_safe(const char *path) {
    if (strstr(path, "..") != NULL)
        return 0;
    if (strstr(path, "//") != NULL)
        return 0;
    if (path[0] == '/')
        return 0;
        
    return 1;
}
void log_request(const char *method, const char *path, int status_code, const char *client_ip) {
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);
    
    FILE *log_file = fopen(LOG_DIR "/access.log", "a");
    if (log_file) {
        fprintf(log_file, "[%s] %s - \"%s %s\" %d\n", 
                timestamp, client_ip, method, path, status_code);
        fclose(log_file);
    }
}
void serve_file(int client_fd, const char *filepath, const char *client_ip) {
    struct stat st;
    if (stat(filepath, &st) == -1) {
        const char *not_found = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><head><title>404 Not Found</title></head>"
            "<body><h1>404 Not Found</h1><p>The requested resource could not be found.</p></body></html>";
        
        write(client_fd, not_found, strlen(not_found));
        log_request("GET", filepath, 404, client_ip);
        return;
    }
    if (S_ISDIR(st.st_mode)) {
        char index_path[BUFFER_SIZE];
        snprintf(index_path, sizeof(index_path), "%s/index.html", filepath);
        
        if (stat(index_path, &st) == -1) {
            const char *dir_listing = 
                "HTTP/1.1 403 Forbidden\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<html><head><title>403 Forbidden</title></head>"
                "<body><h1>403 Forbidden</h1><p>Directory listing not supported.</p></body></html>";
            
            write(client_fd, dir_listing, strlen(dir_listing));
            log_request("GET", filepath, 403, client_ip);
            return;
        }
        serve_file(client_fd, index_path, client_ip);
        return;
    }
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        const char *server_error = 
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><head><title>500 Internal Server Error</title></head>"
            "<body><h1>500 Internal Server Error</h1></body></html>";
        
        write(client_fd, server_error, strlen(server_error));
        log_request("GET", filepath, 500, client_ip);
        return;
    }
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);
    char *content = malloc(filesize);
    if (!content) {
        const char *server_error = 
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><head><title>500 Internal Server Error</title></head>"
            "<body><h1>500 Internal Server Error</h1><p>Memory allocation failed</p></body></html>";
        
        write(client_fd, server_error, strlen(server_error));
        fclose(file);
        log_request("GET", filepath, 500, client_ip);
        return;
    }
    size_t bytes_read = fread(content, 1, filesize, file);
    fclose(file);
    
    if (bytes_read != (size_t)filesize) {
        const char *server_error = 
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><head><title>500 Internal Server Error</title></head>"
            "<body><h1>500 Internal Server Error</h1><p>File read error</p></body></html>";
        
        write(client_fd, server_error, strlen(server_error));
        free(content);
        log_request("GET", filepath, 500, client_ip);
        return;
    }
    const char *content_type = get_content_type(filepath);
    char header[BUFFER_SIZE];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "\r\n", content_type, filesize);
    
    write(client_fd, header, header_len);
    write(client_fd, content, filesize);
    
    free(content);
    log_request("GET", filepath, 200, client_ip);
}
void handle_client(int client_fd, const char *client_ip) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
    
    if (bytes_read <= 0) {
        return;
    }
    
    buffer[bytes_read] = '\0';
    
    char method[16] = {0};
    char path[BUFFER_SIZE] = {0};
    if (sscanf(buffer, "%15s %4095s", method, path) != 2) {
        const char *bad_request = 
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><head><title>400 Bad Request</title></head>"
            "<body><h1>400 Bad Request</h1></body></html>";
        
        write(client_fd, bad_request, strlen(bad_request));
        log_request("INVALID", "INVALID", 400, client_ip);
        return;
    }
    if (strcmp(method, "GET") != 0) {
        const char *method_not_allowed = 
            "HTTP/1.1 405 Method Not Allowed\r\n"
            "Content-Type: text/html\r\n"
            "Allow: GET\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><head><title>405 Method Not Allowed</title></head>"
            "<body><h1>405 Method Not Allowed</h1><p>Only GET method is supported.</p></body></html>";
        
        write(client_fd, method_not_allowed, strlen(method_not_allowed));
        log_request(method, path, 405, client_ip);
        return;
    }
    char clean_path[BUFFER_SIZE] = {0};
    if (path[0] == '/') {
        strncpy(clean_path, path + 1, BUFFER_SIZE - 1);
    } else {
        strncpy(clean_path, path, BUFFER_SIZE - 1);
    }
    if (strlen(clean_path) == 0 || strcmp(clean_path, "/") == 0) {
        char fullpath[BUFFER_SIZE];
        snprintf(fullpath, sizeof(fullpath), "%s/index.html", WEB_ROOT);
        serve_file(client_fd, fullpath, client_ip);
        return;
    }
    if (!is_path_safe(clean_path)) {
        const char *forbidden = 
            "HTTP/1.1 403 Forbidden\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><head><title>403 Forbidden</title></head>"
            "<body><h1>403 Forbidden</h1><p>Invalid request path.</p></body></html>";
        
        write(client_fd, forbidden, strlen(forbidden));
        log_request("GET", clean_path, 403, client_ip);
        return;
    }
    char fullpath[BUFFER_SIZE];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", WEB_ROOT, clean_path);
    serve_file(client_fd, fullpath, client_ip);
}