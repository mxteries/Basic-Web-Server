
all: webserv security
clean:
	-rm -f webserv security data.csv histogram.png

security: security.c
	gcc -Wall -o security security.c
	chmod +x security

webserv: webserv.c
	gcc -Wall -o webserv webserv.c