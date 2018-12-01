#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

enum {
    BUF_SIZE = 256  /* size to hold http requests and responses */
};

// all this does right now is send back the client's request
int handle_request(int client, char* request, ssize_t request_size) {
    // must send a correctly formatted HTTP response or else the browser wont display
    char response[BUF_SIZE];
    snprintf(response, BUF_SIZE, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %li\r\n\r\n", request_size);
    if (send(client, response, strlen(response), 0) == -1) {
        perror("Send error");
        return -1;
    }
    if (send(client, request, request_size, 0) == -1) {
        perror("Send error");
        return -1;
    }
    return 0;
}

// code adapted from APUE textbook (pg 615)
int serve(int server) {
    int client_fd, status;
    pid_t pid;
    for(;;) {
        // accept a new connection (from our local machine)
        // accept will block if no connections are pending
        if ((client_fd = accept(server, NULL, NULL)) < 0) {
            perror("Accept error");
            exit(1);
        }

        // process the client's request by forking a child process
        // I think forking causes the connection to the client to occur twice
        if ((pid = fork()) < 0) {
            perror("fork error");
        } else if (pid == 0) {
            char buf[BUF_SIZE];
            ssize_t n_read = recv(client_fd, buf, BUF_SIZE, 0);
            int ret = handle_request(client_fd, buf, n_read);
            printf("Request handled with return %d\n", ret);
            close(client_fd);
            exit(0); // terminate the child process
        } else {
            /* parent */
            close(client_fd); // close the client fd because the child still maintains a copy
            waitpid(pid, &status, 0);
        }
    }
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
    
    // have our webserver listen to up to 5 connections
    if (listen(webserver, 5) == -1) {
        perror("Listen error");
    } 
    
    serve(webserver);

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
        perror("Server Error");
        exit(1);
    }
    return 0;
}
