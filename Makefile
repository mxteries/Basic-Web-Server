
all: webserv
clean:
		-rm -f webserv

webserv:
	gcc -Wall -o webserv webserv.c