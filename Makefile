all:
	gcc -Wall -c common.c
	gcc -Wall -lpthread user.c common.o -o user
	gcc -Wall -lpthread server.c common.o -o server
