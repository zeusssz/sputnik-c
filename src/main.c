#include "server.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number. Using default port 8080.\n");
        port = 8080;
    }

    printf("Starting Sputnik server on port %d\n", port);
    if (start_server(port) != 0) {
        fprintf(stderr, "Failed to start server on port %d\n", port);
        return 1;
    }

    return 0;
}