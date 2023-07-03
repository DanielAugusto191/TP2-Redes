all:
	gcc -Wall -c common.c
	gcc -Wall -lpthread client.c common.o -o client
	gcc -Wall -lpthread server.c common.o -o server
