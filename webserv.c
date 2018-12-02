#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/* webserv.c implements a simple HTTP 1.1 server 
with short-lived connections using sockets */

enum {               /* constants */
    LOGGING   = 1,   /* if 1, log to terminal, if 0, turn off logging */
    PATH_SIZE = 255, /* size of requested path */  
    BUF_SIZE  = 500  /* size of buffer to hold http requests and responses */
};

// log all requests and responses to terminal
void log_to_stdout(char* data, ssize_t data_size) {
    if (LOGGING) {
        write(STDOUT_FILENO, data, data_size);
    }
}

// all this does right now is send back the client's request
// TODO: determine different content-types
void handle_GET(int client, char* requested_file) {
    char response[BUF_SIZE];
    snprintf(response, BUF_SIZE, "HTTP/1.0 200 OK\r\nContent-Type: unknown\r\nContent-Length: %lu\r\n\r\n%s\n", strlen(requested_file), requested_file);
    int response_len = strlen(response);
    if (send(client, response, response_len, 0) == -1) {
        perror("GET response send error");
    }
    log_to_stdout(response, response_len);
}

// determines the method of the request (eg. GET) and the requested file
// then calls the appropriate function to deal with the request
void handle_request(int client, char* request, ssize_t request_size) {
    log_to_stdout(request, request_size);

    // parse the request by getting the first and second word
    char method[10];
    char request_path[PATH_SIZE];
    if (sscanf(request, "%s %s", method, request_path) < 2) {
        perror("Parsing request failed");
    }

    // deal with the request
    if (strcmp(method, "GET") == 0) {
        handle_GET(client, request_path);
    } else {
        // no other methods have been implemented
    }
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
        if ((pid = fork()) < 0) {
            perror("fork error");
        } else if (pid == 0) {
            char buf[BUF_SIZE];
            ssize_t n_read = recv(client_fd, buf, BUF_SIZE, 0);
            handle_request(client_fd, buf, n_read);
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
    printf("Running on http://127.0.0.1:%u/ (Press CTRL+C to quit)\n", port);

    if (start_server(port) != 0) {
        perror("Server Error");
        exit(1);
    }
    return 0;
}
