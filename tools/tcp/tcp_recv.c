// NAME: recv.c
// DESCRIPTION: receives a file over TCP (stream)
// COMPILE: gcc -Wall -Wextra -O2 tcp_recv.c -o tcp_recv

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define CHUNK_SIZE 1024 // bytes per recv

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <listen_port> <output_filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int listen_port = atoi(argv[1]);
    const char *output_filename = argv[2];

    // Create TCP socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // Allow address reuse (avoids "address already in use" error on restart)
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind to all local interfaces on the given port
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(listen_port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Listen for incoming connections
    if (listen(server_socket, 1) < 0) {
        perror("Listen failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    printf("Listening on port %d...\n", listen_port);

    // Accept one client
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket < 0) {
        perror("Accept failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, sizeof(client_ip));
    printf("Connection accepted from %s:%d\n", client_ip, ntohs(client_addr.sin_port));

    // Open output file for writing binary data
    FILE *outfile = fopen(output_filename, "wb");
    if (!outfile) {
        perror("Failed to open output file");
        close(client_socket);
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Receive data
    char buffer[CHUNK_SIZE];
    ssize_t bytes_received;
    size_t total_received = 0;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_received, outfile);
        total_received += bytes_received;
    }

    if (bytes_received < 0) {
        perror("Receive error");
    }

    printf("Finished receiving %zu bytes.\n", total_received);

    // Cleanup
    fclose(outfile);
    close(client_socket);
    close(server_socket);
    return EXIT_SUCCESS;
}
