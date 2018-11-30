
all: webserv
clean:
	-rm -f webserv

webserv: webserv.c
	gcc -Wall -o webserv webserv.c