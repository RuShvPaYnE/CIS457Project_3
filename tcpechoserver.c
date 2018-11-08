#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>

//tcpechoserver.c

int sockfd;
int clist[10];

void* handleclient(void* arg) {
	int clientsocket = *(int*)arg;
	char line[5000];
	int n = recv(clientsocket, line, 5000, 0);
	printf("\nRecieved from client: %s\n",line);
	if(line[0]=='Q'&&line[1]=='u'&&line[2]=='i'&&line[3]=='t') {
		exit(0);
	}
	//close(clientsocket);
}

void* handleclient2(void* arg) {
	int socket = *(int*)arg;
	char line[5000];
//	scanf("%s", line);
	int m = read(STDIN_FILENO, line, 5000);
	for(int a = 0; a < 5000; a++) {
		if(line[a] == '\n') {
			line[a]=='\0';
		}
	}

	send(socket, line, strlen(line)+1,0);
	if(line[0]=='Q'&&line[1]=='u'&&line[2]=='i'&&line[3]=='t') {
		exit(0);
	}
//	close(socket);
}

void* addclient(void* arg) {
	int clientsocket = *(int*)arg;
	//clist.append(clientsocket);
	int size = 10;
	for(int j = 0; j < size; j++) {
		if((int)clist[j] < 0) {
			clist[j]=clientsocket;
			break;
		}
	}
	while(1) {
		char line[5000];
		int n = recv(clientsocket, line, 5000, 0);
		if(line[0] == '~') {
			char* menu = "Console Opened\n(d#)DM another user, (l)List users, or (k)kill server";
			int m = send(clientsocket, menu, strlen(menu)+1, 0);
			char ans[5];
			recv(clientsocket, ans, 5, 0);
			fflush(stdout);
			fflush(stdin);
			if(ans[0]=='k') {
				//kill server
				char newline[5000];
				snprintf(newline, sizeof(newline), "Quit\0");
				for(int i = 0; i < 10; i++) {
					if(clist[i]>0) {
						send((int)clist[i], newline, strlen(newline)+1, 0);
					}
				}
				//sleep(3);
				exit(0);
			}
			if(ans[0]=='l') {
				for(int x = 0; x < 10; x++) {
					if(clist[x]>0) {
						//printf("%d..%d", x, clist[x]);
						char newline[5000];
						snprintf(newline, sizeof(newline), "%d -- (ID)%d", x, clist[x]);
						send(clientsocket, newline, strlen(newline)+1, 0);
						sleep(1);
					}
				}
			}
			if(ans[0]=='d') {
				//dm someone
				char getdm[5000];
			       	snprintf(getdm, sizeof(getdm), "Enter message to send privately");
				send(clientsocket, getdm, strlen(getdm)+1, 0);
				char dm[5000];
				recv(clientsocket, dm, 5000, 0);
				
				for(int i = 0; i < 10; i++) {
					if(clist[i] == (int)ans[1]-48) {
						send((int)clist[i], dm, strlen(dm)+1, 0);
					}
				}
			}
			if(ans[0] == 'r') {
				char rmsg[5000];
				snprintf(rmsg, sizeof(rmsg), "Quit");

				for(int i = 0; i < 10; i++) {
					if(clist[i]==(int)ans[1]-48) {
						//close(clist[i]);
						//send((int)clist[i], rmsg, strlen(rmsg)+1, 0);
						//recv(clist[i], extra, 5000, 0);
						shutdown(clist[i], SHUT_RDWR);
						close(clist[i]);
						clist[i]=-20;
						return;
					}
				}
			}
		}
		else {
			printf("\n<%d> says: %s", clientsocket,line);
			char newline[5000];
			snprintf(newline, sizeof(newline), "<%d> posts: %s", clientsocket, line);
			for(int i = 0; i < size; i++) {
				if(clist[i]>0) {
					int m = send((int)clist[i], newline, strlen(newline)+1, 0);
					//printf("sent to %d", clist[i]);
					//printf("message sent to %d", clientsocket);
				}
			}
		}
		//recv and send back out?
	}
	close(clientsocket);
}

int main(int argc, char** argv) {

	for(int x = 0; x < 10; x++) {
		clist[x]=-20;
	}

	//creating socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serveraddr, clientaddr;
	serveraddr.sin_family=AF_INET;
	
	//reading in port number
	int portnum;
	printf("Enter port number: ");
	scanf("%d", &portnum);
	//printf("\nListening...\n");

	serveraddr.sin_port=htons(portnum);
	serveraddr.sin_addr.s_addr=INADDR_ANY;	
	bind(sockfd, (struct sockaddr*)&serveraddr,sizeof(serveraddr));
	listen(sockfd,10);

	int len = sizeof(clientaddr);
	//int clientsocket = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
		
	while(1) {
		int clientsocket = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
		//fflush(stdin);
		//fflush(stdout);
		//printf("\ntype away\n");
		//int len = sizeof(clientaddr);
		//int clientsocket = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
		pthread_t child;
		pthread_create(&child, NULL, addclient, &clientsocket);
		pthread_detach(child);
		
		//printf("\ntype away\n");
		//pthread_t child2;
		//pthread_create(&child2, NULL, handleclient2, &clientsocket);
		//pthread_detach(child2);
	//}
	//while(1) {
	}

	close(sockfd);

	return 0;
}
