// UDP_Echo_Client.c
// Created by korfhagem on 3/19/24.
//
// Sends a broadcast message to a particular port.
// Receives and prints any responses.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>


#define UDP_PROTOCOL_NUM 17 // http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
#define ASCII_COLOR_GREEN "\e[0;32m"
#define ASCII_COLOR_RED   "\e[0;31m"
#define ASCII_COLOR_RESET "\e[0m"
#define ASCII_COLOR_YELLO "\e[0;33m"
#define MAX_ETH_PAYLOAD_SIZE 1480

int client_socket_id = 0;


void socket_cleanup( void );
void skt_clean( int option );


void skt_clean( int option ) {
    exit(EXIT_SUCCESS);
}


void socket_cleanup( void ) {
    printf("Closing client socket...\n");
    close(client_socket_id); // cleanup socket on close
}


int main( int argc, char * argv[] ) {

    // register socket cleanup functions for process termination
    atexit(socket_cleanup);
    signal(SIGINT, skt_clean);
    signal(SIGTERM, skt_clean);

    int opt;
    long int port_num = 0;
    char * message = NULL;
    char * ip_addr = "192.168.56.1"; // default send address

    while ((opt = getopt (argc, argv, ":p:m:i:")) != -1) {

        assert(optarg != NULL); // just for sanity

        switch(opt)
        {

            case 'p': { // port option
                char * _rand_ptr;
                port_num = strtol(optarg, &_rand_ptr, 10);
                break;
            }

            case 'm': { // message option
                message = optarg;
                break;
            }

            case 'i': { // ip address option

                ip_addr = optarg;
                break;

            }

            default:
                fprintf(stderr, "Unknown option specified.\nPlease try again. \n");
                exit(EXIT_FAILURE);

        }
    }

    if( port_num == 0 || message == NULL ) {
        fprintf(stderr, "%sNeed to specify port number and message.\nPlease try again.%s\n",
                ASCII_COLOR_RED, ASCII_COLOR_RESET);
        exit(EXIT_FAILURE);
    }

    /*
    ** The client only sends one message. It will send one message, then wait forever in a loop
    ** for responses. Each response it receives will be printed. To send another message, you
    ** would need to re-run the program.
    */

    printf("%sBeginning client on:\nPORT: %ld\nMESSAGE: \"%s\"\n%s",
           ASCII_COLOR_YELLO, port_num, message, ASCII_COLOR_RESET);

    client_socket_id = socket(AF_INET, // IPV4
                                  SOCK_DGRAM, // datagram type
                                  UDP_PROTOCOL_NUM
                                  );

    struct sockaddr_in dest_addr;

    dest_addr.sin_family = AF_INET; // specify ipv4
    dest_addr.sin_port = htons(port_num); // send to port 12345
    dest_addr.sin_addr.s_addr = inet_addr(ip_addr); // specify ip adrr

    // bind socket to port
    bind(client_socket_id, (struct sockaddr *) &dest_addr, sizeof(dest_addr));

    if( dest_addr.sin_addr.s_addr == INADDR_NONE ) { // if ip addr is invalid, exit
        fprintf(stderr, "%sThe ip address \"%s\" couldn't be processed.\nPlease try again.%s\n",
                ASCII_COLOR_RED, ip_addr, ASCII_COLOR_RESET);
        exit(EXIT_FAILURE);
    }

    sendto(client_socket_id, message, strlen(message),
           0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    printf("%sUDP socket successfully created and sent to ip address %s.%s\n",
           ASCII_COLOR_GREEN, ip_addr, ASCII_COLOR_RESET);

    printf("Polling for response...\n...\n");

    time_t current_time;

    #pragma clang diagnostic push
    #pragma ide diagnostic ignored "EndlessLoop"
    while(true) { // listen for messages back

        char receive_buffer[MAX_ETH_PAYLOAD_SIZE] = {0};

        recvfrom(client_socket_id, receive_buffer, MAX_ETH_PAYLOAD_SIZE, 0, NULL, NULL);

        current_time = time(NULL);

        if(strlen(receive_buffer) > 0) { printf("%sReceived message: %s\n",
                                                ctime(&current_time), receive_buffer); }
    }
    #pragma clang diagnostic pop

}