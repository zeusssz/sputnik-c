#include "server.h"

int main(int argc, char *argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    if (start_server(port) != 0) {
        fprintf(stderr, "Failed to start server on port %d\n", port);
        return 1;
    }

    return 0;
}
