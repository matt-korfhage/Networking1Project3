// UDP_Echo_Server.c
// Created by korfhagem on 3/19/24.
//
// • Echoes incoming messages going to a particular port back to the source
// • Appends a custom response to the end of the message
//

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

int server_socket_id = 0;


void socket_cleanup( void );
void skt_clean( int option );


void skt_clean( int option ) {
    exit(EXIT_SUCCESS);
}


void socket_cleanup( void ) {
    printf("Closing client socket...\n");
    close(server_socket_id); // cleanup socket on close
}


int main(int argc, char * argv[]) {

    // register socket cleanup functions for process termination
    atexit(socket_cleanup);
    signal(SIGINT, skt_clean);
    signal(SIGTERM, skt_clean);

    int opt = 0;
    long int port_num = 0;
    char * message = NULL;
    char * ip_addr = "192.168.255.255"; // default send address

    while ((opt = getopt (argc, argv, ":p:m:i:")) != -1) {

        assert(optarg != NULL); // just for sanity

        switch (opt)
        {

            case 'p': { // port option
                char *_rand_ptr;
                port_num = strtol(optarg, &_rand_ptr, 10);
                break;
            }

            case 'm': { // custom message option
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

    printf("%sBeginning server on:\nPORT: %ld\nCUSTOM MESSAGE: \"%s\"\n%s",
           ASCII_COLOR_YELLO, port_num, message, ASCII_COLOR_RESET);

    server_socket_id = socket(AF_INET, // IPV4
                              SOCK_DGRAM, // datagram type
                              UDP_PROTOCOL_NUM
    );

    int broadcast = 1;
    setsockopt(server_socket_id,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof(broadcast));

    struct sockaddr_in bind_addr;

    bind_addr.sin_family = AF_INET; // specify ipv4
    bind_addr.sin_port = htons(port_num); // send to port
    bind_addr.sin_addr.s_addr = inet_addr(ip_addr); // specify ip adrr

    // bind socket to port
    bind(server_socket_id, (struct sockaddr *) &bind_addr, sizeof(bind_addr));

    if(bind_addr.sin_addr.s_addr == INADDR_NONE ) { // if ip addr is invalid, exit
        fprintf(stderr, "%sThe ip address \"%s\" couldn't be processed.\nPlease try again.%s\n",
                ASCII_COLOR_RED, ip_addr, ASCII_COLOR_RESET);
        exit(EXIT_FAILURE);
    }

    printf("%sUDP server successfully created at ip address %s.%s\n",
           ASCII_COLOR_GREEN, ip_addr, ASCII_COLOR_RESET);

    printf("Listening for UDP messages...\n...\n");

    time_t current_time;

    #pragma clang diagnostic push
    #pragma ide diagnostic ignored "EndlessLoop"
    while(true) { // listen for messages back

        char receive_buffer[MAX_ETH_PAYLOAD_SIZE] = {0};

        struct sockaddr_in source_addr;
        socklen_t source_addr_len = sizeof(source_addr);

        recvfrom(server_socket_id, receive_buffer, MAX_ETH_PAYLOAD_SIZE, 0,
                 (struct sockaddr *) &source_addr, &source_addr_len);

        current_time = time(NULL);

        if(strlen(receive_buffer) > 0) {
            printf("%sReceived message from %s: %s\n",
                    ctime(&current_time), inet_ntoa(source_addr.sin_addr), receive_buffer);

            strcat(receive_buffer, message); // append custom response to echo

            sendto(server_socket_id, receive_buffer, strlen(receive_buffer), 0,
                   (struct sockaddr *) &source_addr, source_addr_len);

            printf("Sent \"%s\" back to client at %s\n", receive_buffer, inet_ntoa(source_addr.sin_addr));
        }
    }
    #pragma clang diagnostic pop
}