all: webserv
clean:
	-rm -f webserv

webserv: webserv.c
	gcc -Wall webserv.c -o webserv
