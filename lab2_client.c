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
unsigned char sym_key[32];

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
	if(1 != EVP_DecryptFinal_ex(ctx, plaintext +len,&len))
		handleErrors();
	plaintext_len += len;
	EVP_CIPHER_CTX_free(ctx);
	return plaintext_len;
}

void* handlestuff(void* arg) {
	int socket = *(int*)arg;
	unsigned char line[5000];
	unsigned char D_message[5000];
	int n = recv(socket, line, 5000, 0);

	if(n > 0){
		//BIO_dump_fp (stdout, (const char *)line, 5000);
		decrypt(line,32,sym_key,0,D_message);


		printf("\n%s\n", D_message);
		if(D_message[0]=='Q'&&D_message[1]=='u'&&D_message[2]=='i'&&D_message[3]=='t') {
			shutdown(socket, SHUT_RDWR);
			close(socket);
			exit(0);
		}
	}
	//close(socket);
}

void* handlestuff2(void* arg) {
	int socket = *(int*)arg;
	unsigned char line[5000];
	unsigned char E_message[5000];
	unsigned char Final[5016];
	//scanf("%s", line);
	int m = read(STDIN_FILENO, line, 5000);
	// for(int a = 0; a < 5000; a++) {
	// 	if(line[a]=='\n') {
	// 		line[a]=='\0';
	// 	}
	// }
	unsigned char iv[16];
	RAND_bytes(iv,16);
	if(m > 0){
	int ELength = encrypt(line,strlen((char*)line),sym_key,0,E_message);

	//strcpy((char*)Final,(char*)iv);
	strcpy((char*)Final,(char*)E_message);
	printf("lengt of encrpt, %d",ELength);
	send(socket, E_message, strlen(E_message)+1, 0);
	if(line[0]=='Q'&&line[1]=='u'&&line[2]=='i'&&line[3]=='t') {
		exit(0);
	}
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

	
	unsigned char E_key[256];

	FILE* fp = fopen("RSApub.pem","rb");
	EVP_PKEY *PublicKey;
	PublicKey = PEM_read_PUBKEY(fp,NULL,NULL,NULL);

	RAND_bytes(sym_key,32);
	printf(" sym %u\n",sym_key);
	//BIO_dump_fp (stdout, (const char *)sym_key, 32);
	int encrpytLength = rsa_encrypt(sym_key,32,PublicKey,E_key);
	printf("%d\n",encrpytLength);
	send(sockfd,E_key,256,0);

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
