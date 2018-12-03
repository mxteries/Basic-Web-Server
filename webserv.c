#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <linux/limits.h>

/* webserv.c implements a simple HTTP 1.1 server 
with short-lived connections using sockets 
some code is sourced from the APUE textbook (3rd edition) 
*/

enum {               /* constants */
    LOGGING   = 1,   /* if 1, log to terminal, if 0, turn off logging */
    BUF_SIZE  = 500, /* size of buffer to hold http requests and responses */
    SMALL_BUF = 20   /* for things like method names, status messages, etc. */
};

// log all requests and responses to terminal
void log_to_stdout(char* data, ssize_t data_size) {
    if (LOGGING) {
        write(STDOUT_FILENO, data, data_size);
    }
}

// returns the current date like: Fri, 31 Dec 1999 23:59:59 GMT
// for the HTTP date header
// source: https://stackoverflow.com/a/7548846
void get_date(char* buf, int date_size) {
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(buf, date_size, "%a, %d %b %Y %H:%M:%S %Z", &tm);
}

// code adapted from https://stackoverflow.com/a/5309508
char get_file_extension(char* filename) {
    char *dot = strrchr(filename, '.');
    if (dot == NULL || dot == filename) 
        return 0;
    return *(dot + 1);
}

/**
 * this function can be used to create and format an HTTP 1.1 response
 * the content of the response must be sent separately
 * buf: response is stored in buf (size BUF_SIZE)
 * status: HTTP status code (eg. 200, 404, 501) 
 * type: content type (eg. text/plain)
 */  
void create_response(char* buf, int size, int status, char* type) {
    char date[BUF_SIZE];
    get_date(date, BUF_SIZE);

    // assign a status message to go with the status code
    char status_msg[SMALL_BUF];
    if (status == 200) {
        strcpy(status_msg, "OK");
    } else if (status == 404) {
        strcpy(status_msg, "Not Found");
    } else if (status == 501) {
        strcpy(status_msg, "Not Implemented");
    } else {
        strcpy(status_msg, "Unknown");
    }

    char* response = "HTTP/1.1 %d %s\r\nDate: %s\r\nConnection: close\r\nContent-Type: %s\r\n\r\n";
    snprintf(buf, size, response, status, status_msg, date, type);
}

// log a response, and send it to the client
void send_response(int client, char* response) {
    int response_len = strlen(response);
    log_to_stdout(response, response_len);
    if (send(client, response, response_len, 0) == -1) {
        perror("Send failed");
    }
}

// file not found error
void send_404(int client) {
    char* err_msg = "The requested item could not be found";
    char response[BUF_SIZE];
    create_response(response, BUF_SIZE, 404, "text/plain");
    send_response(client, response);
    send(client, err_msg, strlen(err_msg), 0); // send the contents
}

// method not implemented error
void send_501(int client) {
    char* err_msg = "The requested item has no implementation for it";
    char response[BUF_SIZE];
    create_response(response, BUF_SIZE, 501, "text/plain");
    send_response(client, response);
    send(client, err_msg, strlen(err_msg), 0); // send the contents
}

// lists a directory and sends it to the client
void handle_dir(int client, char* dir_name) {
    // send the initial response head
    char response[BUF_SIZE];
    create_response(response, BUF_SIZE, 200, "text/plain");
    send_response(client, response);

    // popen an ls -l process, read from the pipe and send it to the client
    // adapted from APUE book (pg 615-616)
    FILE* fp;
    char ls_entry[BUF_SIZE];
    char cmd[NAME_MAX];
    sprintf(cmd, "ls -l %s", dir_name); // format ls -l on dir_name

    if ((fp = popen(cmd, "r")) == NULL) {
        perror("Popen ls -l error");
    } else {
        // read a line from the ls -l process and send it to the client
        while (fgets(ls_entry, BUF_SIZE, fp) != NULL) {
            send(client, ls_entry, strlen(ls_entry), 0);
        }
        pclose(fp);
    }
}

// TODO: handle regular files
// if file exists, it is either a directory or a regular file
// if directory, call ls -l on it
// if reg file, determine file extension (eg. .html, .py, .jpg, etc.)
void handle_GET(int client, char* requested_file) {
    struct stat sb;
    if (stat(requested_file, &sb) == -1) {
        /* if stat fails, assume file doesn't exist */
        perror("stat");
        send_404(client);
    } else if (S_ISDIR(sb.st_mode)) {
        /* handle directory file */
        handle_dir(client, requested_file);
    } else if (S_ISREG(sb.st_mode)) {
        /* handle regular file */
        send_501(client);
    } else {
        /* unknown file type */
        send_501(client);
    }

    // if its a regular file, it is either a .html, .jpg/.jpeg/.gif, a .cgi, or a .py file
    // it it's not one of those, return a 501 error
}

// determines the method of the request (eg. GET) and the requested file
// then calls the appropriate function to deal with the request
void handle_request(int client, char* request, ssize_t request_size) {
    log_to_stdout(request, request_size);

    // parse the request by getting the first and second word
    char method[SMALL_BUF];
    char temp_path[PATH_MAX];
    if (sscanf(request, "%s %s", method, temp_path) < 2) {
        perror("Parsing request failed");
    }

    // add a "." before the requested path
    char relative_path[PATH_MAX];
    sprintf(relative_path, ".%s", temp_path);

    // deal with the request
    if (strcmp(method, "GET") == 0) {
        handle_GET(client, relative_path);
    } else {
        // no other methods have been implemented
        send_501(client);
    }
}

// code adapted from APUE textbook (pg 617-618)
// accept connections and fork a child process to serve requests
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
            exit(1);
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

    // connect address and webserver together using bind
    if (bind(webserver, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
        perror("Bind error");
        return 1;
    }
    
    // have our webserver listen to up to 5 connections
    if (listen(webserver, 5) == -1) {
        perror("Listen error");
        return 1;
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
        exit(1);
    }
    return 0;
}
