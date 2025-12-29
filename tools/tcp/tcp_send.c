// NAME: tcp_send.c
// DESCRIPTION: Sends a file over TCP (stream)
// COMPILE: gcc -Wall -Wextra -O2 tcp_send.c -o tcp_send


#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define CHUNK_SIZE 1024 // bytes per send

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <peer_ip> <peer_port> <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *peer_ip = argv[1];
    int peer_port = atoi(argv[2]);
    const char *filename = argv[3];

    // Create TCP socket
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // Configure peer address
    struct sockaddr_in peer_addr = {0};
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(peer_port);

    if (inet_pton(AF_INET, peer_ip, &(peer_addr.sin_addr)) <= 0) {
        perror("Invalid IP address");
        close(tcp_socket);
        return EXIT_FAILURE;
    }

    // Connect to receiver
    printf("Connecting to %s:%d...\n", peer_ip, peer_port);
    if (connect(tcp_socket, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
        perror("Connection failed");
        close(tcp_socket);
        return EXIT_FAILURE;
    }

    // Open file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        close(tcp_socket);
        return EXIT_FAILURE;
    }

    printf("Sending file \"%s\" to %s:%d...\n", filename, peer_ip, peer_port);

    // Send file data
    char buffer[CHUNK_SIZE];
    size_t bytes_read;
    size_t total_sent = 0;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        ssize_t sent = send(tcp_socket, buffer, bytes_read, 0);
        if (sent < 0) {
            perror("Send failed");
            break;
        }
        total_sent += sent;
    }

    printf("Finished sending %zu bytes.\n", total_sent);

    // Cleanup
    fclose(file);
    close(tcp_socket);
    return EXIT_SUCCESS;
}
