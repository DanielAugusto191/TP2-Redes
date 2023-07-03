#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFSZ 2000
#define MAX_CONN 2

void *client_thread(void *data);
struct client_data{
	int id;
	int csock;
	struct sockaddr_storage storage;
};

struct client_data client_list[MAX_CONN];
int client_check[MAX_CONN];

int main(int argc, char *argv[]){

	struct sockaddr_storage storage;
	if(0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
		DieWithSystemMessage("Erro no parser!");
	}

	// Socket
	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if(s == -1){
		DieWithSystemMessage("Erro ao abrir socket!");
	}
	int enable = 1;
	if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
		DieWithSystemMessage("Erro setsocket!");
	}

	struct sockaddr *addr = (struct sockaddr *)(&storage);

	//bind
	if(0 != bind(s, addr, sizeof(storage))){
		DieWithSystemMessage("Bind error!");
	}

	//listen
	if(0 != listen(s, 1)) {
		DieWithSystemMessage("listen error!");
	}
	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);
	memset(client_check, 0, MAX_CONN*sizeof(int));
	while(1){
		//accept
		struct sockaddr_storage cstorage;
		struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
		socklen_t caddrlen = sizeof(cstorage);
		int csock = accept(s, caddr, &caddrlen);

		if(csock == -1) DieWithSystemMessage("Accept error!");
		
		struct client_data *cdata = malloc(sizeof(*cdata));
		if(!cdata) DieWithSystemMessage("malloc error!");
		cdata->csock = csock;
		memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));
		int i =0;
		while(i < MAX_CONN && client_check[i] == 1) ++i;
		if(i == MAX_CONN){ // MAX CON
			char msg[] = "00";
			size_t count = send(cdata->csock, msg, strlen(msg), 0);
			if(count != strlen(msg)) DieWithUserMessage("erro: ", "max conn erro!");
		}else{
			cdata->id = i;
			client_check[i] = 1;
			client_list[i] = *cdata;
			char msg[3];
			sprintf(msg, "%02d", i+1);
			printf("User %02d added\n", i+1);
			size_t count = send(cdata->csock, msg, strlen(msg), 0);
			if(count != strlen(msg)) DieWithUserMessage("erro: ", "confirm conn error");
			pthread_t tid;
			pthread_create(&tid, NULL, client_thread, cdata);
		}
	}
	exit(EXIT_SUCCESS);
}

void *client_thread(void *data){
	struct client_data *cdata = (struct client_data *)data;

	char buf[BUFSZ];
	while(1){
		memset(buf, 0, BUFSZ);
		unsigned total = 0;
		while(1){
			size_t count = recv(cdata->csock, buf + total, BUFSZ - total, 0);
			if(count < 0) DieWithSystemMessage("Error!");
			total += (int)count;
			if(count <= 0) break;
			if(buf[count] == '\0') break;
		}
		if(total == 0) break;
		if(!strncmp(buf, "close connection", 16)){
			char ans[] = "confirm";
			size_t count = send(cdata->csock, ans, sizeof(ans), 0);
			if(count != sizeof(ans)) DieWithSystemMessage("bytes erro!");
			break;
		}else if(!strncmp(buf, "list users", 9)){
			char msg[50];
			strcpy(msg, "");
			for(int i=0;i<MAX_CONN;++i) if(client_check[i] == 1){
				char t[5];
				char spc = ' ';
				sprintf(t, "%02d", i+1);
				strncat(t, &spc, 1);
				strcat(msg, t);
			}
			msg[strlen(msg)-1] = '\0'; // remove extra ' '
			size_t count = send(cdata->csock, msg, strlen(msg), 0);
			if(count != strlen(msg)) DieWithSystemMessage("bytes erro!");
		}else if(buf[0] == 's' && buf[1] == 'e' && buf[2] == 'n' && buf[3] == 'd'){
			if(buf[5] == 'a'){
				char msg[BUFSZ];
				strcpy(msg, buf+10);
				msg[strlen(msg)-2] = '\0';
				time_t now = time(NULL);
				struct tm *tm_struct = localtime(&now);
				int hours = tm_struct->tm_hour, minutes = tm_struct->tm_min;
				printf("[%d:%d] %02d: %s\n", hours, minutes, cdata->id+1, msg);

				char msgback[BUFSZ];
				sprintf(msgback, "[%d:%d] -> all: %.1000s", hours, minutes, msg);
				size_t count = send(cdata->csock, msgback, strlen(msgback), 0);
				if(count != strlen(msgback)) DieWithSystemMessage("bytes erro!");

				char msgall[BUFSZ];
				sprintf(msgall, "[%d:%d] %02d: %.1000s", hours, minutes, cdata->id+1, msg);
				for(int i=0;i<MAX_CONN;++i) if(client_check[i]){
					if(i == cdata->id) continue;
					size_t count = send(client_list[i].csock, msgall, strlen(msgall), 0);
					if(count != strlen(msgall)) DieWithSystemMessage("bytes erro!");
				}

				
			}else{
				char sid[3];
				sid[0] = buf[8];
				sid[1] = buf[9];
				int id = atoi(sid);
				--id; // 0-based
				char msg[BUFSZ];
				strcpy(msg, buf+12);
				msg[strlen(msg)-2] = '\0'; // remove last "
				if(client_check[id] == 0){
					printf("User %02d not found\n", id+1);
					char msg[2] = "-1";
					size_t count = send(cdata->csock, msg, strlen(msg), 0);
					if(count != strlen(msg)) DieWithSystemMessage("not send user not found!");
				}else{
					time_t now = time(NULL);
					struct tm *tm_struct = localtime(&now);
					int hours = tm_struct->tm_hour, minutes = tm_struct->tm_min;
					
					char msgback[BUFSZ];
					sprintf(msgback, "P [%d:%d] -> %02d: %.1000s", hours, minutes, cdata->id+1, msg);
					size_t count = send(cdata->csock, msgback, strlen(msgback), 0);
					if(count != strlen(msgback)) DieWithSystemMessage("bytes erro!");

					char msgto[BUFSZ];
					sprintf(msgto, "P [%d:%d] %02d: %.1000s", hours, minutes, id+1, msg);
					size_t count2 = send(client_list[id].csock, msgto, strlen(msgto), 0);
					if(count2 != strlen(msgto)) DieWithSystemMessage("bytes erro!");
				}
			}
		}else{
			puts(buf);
		}
	}
	client_check[cdata->id] = 0;
	close(cdata->csock);
	pthread_exit(EXIT_SUCCESS);
}
