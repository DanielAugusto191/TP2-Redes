#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 2048

void *listen_thread(void *data);

struct list_sock {
	int s;
};
int mutex, closed;

int main(int argc, char **argv){
	if(argc < 3) DieWithUserMessage("Fail!", "menos args");
	
	struct sockaddr_storage storage;
	if(0 != addrparse(argv[1], argv[2], &storage)) {
		DieWithSystemMessage("Erro no parser!");
	}

	// Socket
	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if(s == -1){
		DieWithSystemMessage("Erro ao abrir socket!");
	}
	mutex = 0;

	char buf[BUFSZ];
	
	// Connect
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) DieWithSystemMessage("Erro ao connectar!");
	closed = 0;
	memset(buf, 0, BUFSZ);
	unsigned total = 0;
	while(1){
		size_t count = recv(s, buf + total, BUFSZ - total, 0);
		if(count < 0) DieWithSystemMessage("Error!");
		total += (int)count;
		if(count <= 0) break;
		if(buf[count] == '\0') break;
	}
	if(!strncmp(buf, "00", 2)) printf("User limit exceeded\n");
	else{
		printf("%s\n", buf);
		struct list_sock *ls = malloc(sizeof(*ls));
		ls->s = s;
		pthread_t tid;
		pthread_create(&tid, NULL, listen_thread, ls);
		while(1){
			// Send msg
			memset(buf, 0, BUFSZ);
			mutex = 0;
			fgets(buf, BUFSZ-1, stdin);
			mutex = 1;
			if(!strncmp(buf, "close connection", 16)){
				closed = 1;
				size_t count = send(s, buf, strlen(buf), 0);
				if(count != strlen(buf)) DieWithSystemMessage("Erro ao enviar!");
				memset(buf, 0, BUFSZ);
				unsigned total = 0;
				while(1){
					size_t count = recv(s, buf + total, BUFSZ - total, 0);
					if(count < 0) DieWithSystemMessage("Error!");
					total += (int)count;
					if(count <= 0) break;
					if(buf[count] == '\0') break;
				}
				if(!strncmp(buf, "confirm", 7)) {
					printf("Removed Successfully\n");
					break;
				}
				
			}else if(!strncmp(buf, "list users", 10)){
				size_t count = send(s, buf, strlen(buf), 0);
				if(count != strlen(buf)) DieWithSystemMessage("Erro ao enviar!");
				memset(buf, 0, BUFSZ);
				unsigned total = 0;
				while(1){
					size_t count = recv(s, buf + total, BUFSZ - total, 0);
					if(count < 0) DieWithSystemMessage("Error!");
					total += (int)count;
					if(count <= 0) break;
					if(buf[count] == '\0') break;
				}
				printf("%s\n", buf);
			}else if(buf[0] == 's' && buf[1] == 'e' && buf[2] == 'n' && buf[3] == 'd'){
				if(buf[5] == 'a'){
					size_t count = send(s, buf, strlen(buf), 0);
					if(count != strlen(buf)) DieWithSystemMessage("Erro ao enviar!");
					memset(buf, 0, BUFSZ);
					unsigned total = 0;
					while(1){
						size_t count = recv(s, buf + total, BUFSZ - total, 0);
						if(count < 0) DieWithSystemMessage("Error!");
						total += (int)count;
						if(count <= 0) break;
						if(buf[count] == '\0') break;
					}
					printf("%s\n", buf);
				}else{
					size_t count = send(s, buf, strlen(buf), 0);
					if(count != strlen(buf)) DieWithSystemMessage("Erro ao enviar!");
					memset(buf, 0, BUFSZ);
					unsigned total = 0;
					while(1){
						size_t count = recv(s, buf + total, BUFSZ - total, 0);
						if(count < 0) DieWithSystemMessage("Error!");
						total += (int)count;
						if(count <= 0) break;
						if(buf[count] == '\0') break;
					}
					if(!strncmp(buf, "-1", 2)) printf("Receiver not found\n");
					else printf("%s\n", buf);
				}
			}else{
			}
		}
	}
	close(s);
	exit(EXIT_SUCCESS);
}

void *listen_thread(void *data){
	struct list_sock *ls = (struct list_sock *) data;
	char buf[BUFSZ];
	while(1){
		memset(buf, 0, BUFSZ);
		unsigned total = 0;
		while(1){
			while(mutex);
			size_t count = recv(ls->s, buf + total, BUFSZ - total, 0);
			if(count < 0) DieWithSystemMessage("Error!");
			total += (int)count;
			if(count <= 0) break;
			if(buf[count] == '\0') break;
			if(closed) break;
		}
		printf("%s\n", buf);
		if(closed) break;
	}
	pthread_exit(EXIT_SUCCESS);
}

