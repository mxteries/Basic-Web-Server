#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int sockfd;

int start_server(char *port) {
    struct addrinfo hints, *res, *p;

    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    // get address for server socket to bind to (and be reachable from)
    if (getaddrinfo( NULL, port, &hints, &res) != 0)
    {
        printf("Error: unable to get address info\n");
        exit(1);
    }
    
    // apparently we have to loop through list of all potential interfaces, until socket setup is succesful
    for (p = res; p != NULL; p = p->ai_next) {
        // create a socket
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            printf("Error: unable to create socket\n");
            continue;
        }
        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            close(sockfd);
            freeaddrinfo(res); // all done with this structure
            exit(1);
        }
        if ((bind(sockfd, res->ai_addr, res->ai_addrlen)) == -1) {
            printf("Error: unable to bind socket to address");
            close(sockfd);
            continue;
        }
        printf("Succesfully setup socket\n");
        break;
    }
    
    // sets the socket as an active server
    if ( listen (sockfd, 100) != 0 )
    {
        printf("Error: listen failed");
        exit(1);
    }

    return 1;
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Error: to start server use: $./webserv port-number\n");
        return 0;
    }

    char port[6];
    strcpy(port, argv[1]);
    if (start_server(port) != 1) {
        printf("Error: server unable to start");
        exit(1);
    }
}
