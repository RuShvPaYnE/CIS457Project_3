#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <string.h>

//lab2_client.c

void handleErrors(void) {
	ERR_print_errors_fp(stderr);
	abort();
}

int rsa_encrypt(unsigned char* in, size_t inlen, EVP_PKEY *key, unsigned char* out) {
	EVP_PKEY_CTX *ctx;
	size_t outlen;
	ctx = EVP_PKEY_CTX_new(key, NULL);
	if(!ctx)
		handleErrors();
	if(EVP_PKEY_encrypt_init(ctx) <= 0)
		handleErrors();
	if(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
		handleErrors();
	if(EVP_PKEY_encrypt(ctx, NULL, &outlen, in, inlen) <=0 )
		handleErrors();
	if(EVP_PKEY_encrypt(ctx, out, &outlen, in, inlen) <=0 )
		handleErrors();
	return outlen;
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext) {
	EVP_CIPHER_CTX *ctx;
	int len;
	int ciphertext_len;
	if(!(ctx = EVP_CIPHER_CTX_new()))
		handleErrors();
	if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
		handleErrors();
	if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
		handleErrors();
	ciphertext_len = len;
	if(1 != EVP_EncryptFinal_ex(ctx, ciphertext+len, &len))
		handleErrors();
	ciphertext_len += len;
	EVP_CIPHER_CTX_free(ctx);
	return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext) {
	EVP_CIPHER_CTX *ctx;
	int len;
	int plaintext_len;
	if(!(ctx = EVP_CIPHER_CTX_new()))
		handleErrors();
	if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
		handleErrors();
	if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
		handleErrors();
	plaintext_len = len;
	if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
		handleErrors();
	plaintext_len += len;
	EVP_CIPHER_CTX_free(ctx);
	return plaintext_len;
}

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
