// NAME: udpsend.c


#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char * argv[]){
    if (argc != 4){
        printf("Usage: %s <peer_ip> <peer_port> <message>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // get some information about our peer form out args
    const char * peer_ip = argv[1];
    int peer_port = atoi(argv[2]);
    const char * message = argv[3];

    struct sockaddr_in peer_addr = {.sin_family = AF_INET, .sin_port = htons(peer_port)};

    if (inet_pton(AF_INET, peer_ip, &(peer_addr.sin_addr)) <= 0) {
        perror ("Something wrong with the IP Address!");
        return EXIT_FAILURE;
    }

    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket <0){
        perror("Sorry, couldnt create the socket.");
        return EXIT_FAILURE;
    }

    ssize_t sent = sendto(udp_socket, message, strlen(message) + 1, 0,
                        (struct sockaddr *)&peer_addr, sizeof(peer_addr));
    if (sent < 0) {
        perror("Failed to send message.");
    } else {
        printf("Sent %zd bytes successfully\n", sent);
    }

    printf("sent \"%s\" to %s:%d\n", message, peer_ip, peer_port);
    close(udp_socket);

    return EXIT_SUCCESS;
}

