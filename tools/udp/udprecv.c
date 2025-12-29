// NAME: udprecv.c

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int check(int exp, const char *msg);

int main(int argc, char **argv){

    if (argc != 2) {
        printf("usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int my_port = atoi(argv[1]);
    int udp_rx_socket;
    struct sockaddr_in peer_addr;
    struct sockaddr_in my_addr = {.sin_family = AF_INET,
                                    .sin_addr = INADDR_ANY,
                                    .sin_port = htons(my_port)};

    char buffer[BUFFER_SIZE];

    if ((udp_rx_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {
        perror("Failed to create socket!");
        return EXIT_FAILURE;
    }

    //bind the socekt to the address/port
    int result = bind(udp_rx_socket, (struct sockaddr *)&my_addr, sizeof(my_addr));
    check(result, "Could not bind socket to address.");

    socklen_t address_length = sizeof(peer_addr);
    int bytes_received = recvfrom(udp_rx_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&peer_addr, &address_length);
    check(bytes_received, "Sorry, recvfrom failed.");
    
    printf("received a packet from %s:%d -- Message = %s\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port), buffer);

    close(udp_rx_socket);

    return EXIT_SUCCESS;
}

#define SOCKETERROR (-1)
int check(int exp, const char *msg)
{
    if (exp == SOCKETERROR){
        perror(msg);
        exit(EXIT_FAILURE);
    }

    return exp;
}