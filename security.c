#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
//FROM LAB 6
int uart_open(char* path, speed_t baud){
    struct termios uart_opts;
    int fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);
    // Flush data already in/out
    if (tcflush(fd,TCIFLUSH)==-1)
        goto err;
    if (tcflush(fd,TCOFLUSH)==-1)
        goto err;
    // Setup modes (8-bit data, disable control signals, readable, no-parity)
    uart_opts.c_cflag = CS8 | CLOCAL | CREAD;    // control modes
    uart_opts.c_iflag=IGNPAR;                           // input modes
    uart_opts.c_oflag=0;                                // output modes
    uart_opts.c_lflag=0;                                // local modes
    // Setup input buffer options: Minimum input: 1byte, no-delay
    uart_opts.c_cc[VMIN]=1;
    uart_opts.c_cc[VTIME]=0;
    // Set baud rate
    cfsetospeed(&uart_opts,baud);
    cfsetispeed(&uart_opts,baud);
    // Apply the settings
    if (tcsetattr(fd,TCSANOW,&uart_opts)==-1)
        goto err;
    
    return fd;
    
err:
    close(fd);
    return -1;
}

int main(int argc, char** argv){
    char buffer[1024];
    int uart;
    
    if(argc < 2){
        printf("Usage: serial-dev-path");
        return EXIT_SUCCESS;
    }
    printf("Setting up\n");
        uart = uart_open(argv[1], B57600);
        if(uart < 0){
            printf("Error: COuld not open %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    printf("Connection Confirmed\n");
        memset(buffer, 0, sizeof(buffer));
        while (1){
            // Put code here to update website!
            read(uart, buffer, sizeof(buffer));
            printf("%s", buffer);
            if (strcmp(buffer, "breach") == 0) {
                printf("Alert! Your security system has been breached!");
            }
            memset(buffer, 0, sizeof(buffer));
        }
        // Connection is closed/lost, close the file and exit
        close(uart);
        return EXIT_SUCCESS;
        
    }
