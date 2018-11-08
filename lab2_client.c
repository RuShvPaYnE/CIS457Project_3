#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

//lab2_client.c

void* handlestuff(void* arg) {
	int socket = *(int*)arg;
	char line[5000];
	int n = recv(socket, line, 5000, 0);
	printf("\n%s\n", line);
	if(line[0]=='Q'&&line[1]=='u'&&line[2]=='i'&&line[3]=='t') {
		shutdown(socket, SHUT_RDWR);
		close(socket);
		exit(0);
	}
	//close(socket);
}

void* handlestuff2(void* arg) {
	int socket = *(int*)arg;
	char line[5000];
	//scanf("%s", line);
	int m = read(STDIN_FILENO, line, 5000);
	for(int a = 0; a < 5000; a++) {
		if(line[a]=='\n') {
			line[a]=='\0';
		}
	}


	send(socket, line, strlen(line)+1, 0);
	if(line[0]=='Q'&&line[1]=='u'&&line[2]=='i'&&line[3]=='t') {
		exit(0);
	}
	//close(socket);
}

int main(int argc, char** argv){

	//creating socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0){
		printf("There was an error creating the socket\n");
		return 1;
	}

	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;

	//reading port number
	printf("Please enter a port number: ");
	int portnum;
	scanf("%d", &portnum);
	serveraddr.sin_port=htons(portnum);
	
	//reading in I.P.
	char ipaddr[100];
	printf("Please enter the I.P. address: ");
	scanf("%s", ipaddr);
	serveraddr.sin_addr.s_addr=inet_addr(ipaddr);

	//connecting
	int e = connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

	if(e < 0){
		printf("There was an error connecting\n");
		return 2;
	}
	//while(1) {
		//find file to send
		//printf("Enter a message: ");
		//char message[5000]
		//scanf("%s", message);
	
		//char* message = malloc(sizeof(char)*5000);
		//FILE* fp;

		//make sure file exists
		//while((fp = fopen(filename, "r")) == (FILE*)NULL) {
		//	printf("Filename does not exist, try again: \n");
		//	scanf("%s", filename);
		//}
		
		//sending file
		//fgets(message, 5000, fp);
		//send(sockfd, message, strlen(message)+1, 0);
		//free(message);
		//fclose(fp);

		//recieving file
		//char recmessage[5000];
		//recv(sockfd, recmessage, 5000, 0);
		//FILE* fp2;

		//writing message to new file
		//fp2 = fopen("recievedMessage", "w");
		//fprintf(fp2, "%s", recmessage);
		//fclose(fp2);
		//printf("recieved from server: %s\n", recmessage);
	//}
	//close socket
	while(1) {
	
		fflush(stdin);
		fflush(stdout);
	
		pthread_t child;
		pthread_create(&child, NULL, handlestuff, &sockfd);
		pthread_detach(child);

		pthread_t tchild;
		pthread_create(&tchild, NULL, handlestuff2, &sockfd);
		pthread_detach(tchild);
		
	}

	close(sockfd);

	return 0;
}
