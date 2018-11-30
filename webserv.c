#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// all this does is send a message to the client right now
int handle_request(int client) {
    // write to the client_socket
    char message[] = "You've connected to the server";
    return send(client, message, strlen(message), 0);
}

int start_server(uint16_t port) {
    // create a socket for our webserver
    int webserver;
    webserver = socket(AF_INET, SOCK_STREAM, 0);

    // set up the address of the server
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // we defined an address for our webserver, 
    // now let's connect them together using bind
    if (bind(webserver, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
        perror("Bind error");
    }
    listen(webserver, 5); // have our webserver listen to up to 5 connections
    
    // accept a new connection (from our local machine)
    int client_socket = accept(webserver, NULL, NULL);
    printf("Connection accepted for client %d\n", client_socket);

    // process the client's request by forking a child process
    if (fork() == 0) {
        printf("Child process running request for client %d\n", client_socket);
        handle_request(client_socket);
    }

    // might want to set up a signal handler for sigint to call close
    close(webserver);
    return 0;
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Error: to start server use: $./webserv port-number\n");
        exit(1);
    }

    uint16_t port = (uint16_t) strtol(argv[1], NULL, 10);
    printf("Starting server on port %u\n", port);
    if (start_server(port) != 0) {
        perror("Error: ");
        exit(1);
    }
    return 0;
}
