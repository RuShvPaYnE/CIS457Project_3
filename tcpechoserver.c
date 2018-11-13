#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <string.h>

//tcpechoserver.c

int sockfd;
int clist[10];
unsigned char kList[10][32];

EVP_PKEY *PrivateKey;

void handleErrors(void)
{
	ERR_print_errors_fp(stderr);
	abort();
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
	EVP_CIPHER_CTX *ctx;
	int len;
	int ciphertext_len;
	if (!(ctx = EVP_CIPHER_CTX_new()))
		handleErrors();
	if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
		handleErrors();
	if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
		handleErrors();
	ciphertext_len = len;
	if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
		handleErrors();
	ciphertext_len += len;
	EVP_CIPHER_CTX_free(ctx);
	return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
	EVP_CIPHER_CTX *ctx;
	int len;
	int plaintext_len;
	if (!(ctx = EVP_CIPHER_CTX_new()))
		handleErrors();
	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
		handleErrors();
	if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
		handleErrors();
	plaintext_len = len;
	if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
		handleErrors();
	EVP_CIPHER_CTX_free(ctx);
	return plaintext_len;
}

int rsa_decrypt(unsigned char *in, size_t inlen, EVP_PKEY *key, unsigned char *out)
{
	EVP_PKEY_CTX *ctx;
	size_t outlen;
	ctx = EVP_PKEY_CTX_new(key, NULL);
	if (!ctx)
		handleErrors();
	if (EVP_PKEY_decrypt_init(ctx) <= 0)
		handleErrors();
	if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
		handleErrors();
	if (EVP_PKEY_decrypt(ctx, NULL, &outlen, in, inlen) <= 0)
		handleErrors();
	if (EVP_PKEY_decrypt(ctx, out, &outlen, in, inlen) <= 0)
		handleErrors();
	return outlen;
}

void *handleclient(void *arg)
{
	int clientsocket = *(int *)arg;
	char line[5016];
	char temp[5000];
	char D_message[5000];
	int n = recv(clientsocket, line, 5016, 0);
	unsigned char tempKey[32];
	for (int i = 0; i < 10; i++)
	{
		if (clist[i] == clientsocket)
		{
			snprintf(tempKey, sizeof(tempKey), kList[i]);
		}
	}
	if (n > 0)
	{
		unsigned char iv[16];
		strncpy(iv, line, 16);
		int x =strlen(line) - 5000;
		snprintf(temp, sizeof(tempKey), line[x]);
		decrypt(temp, 16, tempKey, iv, D_message);

		printf("\nRecieved from client: %s\n", D_message);
		if (D_message[0] == 'Q' && D_message[1] == 'u' && D_message[2] == 'i' && D_message[3] == 't')
		{
			exit(0);
		}
	}
	//close(clientsocket);
}

void *handleclient2(void *arg)
{
	int socket = *(int *)arg;
	char line[5000];
	char E_message[7000];
	char Final[5016];
	//	scanf("%s", line);
	int m = read(STDIN_FILENO, line, 5000);
	if (m > 0)
	{
		for (int a = 0; a < 5000; a++)
		{
			if (line[a] == '\n')
			{
				line[a] == '\0';
			}
		}

		unsigned char tempKey[32];
		for (int i = 0; i < 10; i++)
		{
			if (clist[i] == socket)
			{
				snprintf(tempKey, sizeof(tempKey), kList[i]);
			}
		}

		unsigned char iv[16];
		RAND_bytes(iv, 16);

		encrypt(line, strlen((char *)line), tempKey, 0, E_message);

		send(socket, line, strlen(line) + 1, 0);
		if (line[0] == 'Q' && line[1] == 'u' && line[2] == 'i' && line[3] == 't')
		{
			exit(0);
		}
	}
	//	close(socket);
}

void *addclient(void *arg)
{
	int clientsocket = *(int *)arg;
	unsigned char dKey[32];
	unsigned char key[256];
	printf("getting key\n");
	recv(clientsocket, key, 256, 0);
	int decryptLength = rsa_decrypt(key, 256, PrivateKey, dKey);
	printf("got the key maybe\n");
	//clist.append(clientsocket);
	int size = 10;
	for (int j = 0; j < size; j++)
	{
		if ((int)clist[j] < 0)
		{
			clist[j] = clientsocket;
			kList[j][0] = &dKey;

			break;
		}
	}
	printf("\nready\n");
	while (1)
	{
		unsigned char line[5000];
		unsigned char E_message[5000];
		//int n = recv(clientsocket, line, 5016, 0);
		//printf("HERE");
		int n = recv(clientsocket, E_message, 5000, 0);
		unsigned char tempKey[32];
		//for (int i = 0; i < 10; i++)
		//{
		//	if (clist[i] == socket)
		//	{
		//		snprintf(tempKey, sizeof(tempKey), kList[i][0]);
		//	}
		//}
		decrypt(E_message, 16, dKey, 0,line);
		//decrypt(line, strlen(line), tempKey, iv, E_message);
		if (line[0] == '~')
		{
			char *menu = "Console Opened\n(d#)DM another user, (l)List users, or (k)kill server";
			int m = send(clientsocket, menu, strlen(menu) + 1, 0);
			char ans[5];
			recv(clientsocket, ans, 5, 0);
			fflush(stdout);
			fflush(stdin);
			if (ans[0] == 'k')
			{
				//kill server
				char newline[5000];
				snprintf(newline, sizeof(newline), "Quit\0");
				for (int i = 0; i < 10; i++)
				{
					if (clist[i] > 0)
					{
						send((int)clist[i], newline, strlen(newline) + 1, 0);
					}
				}
				//sleep(3);
				exit(0);
			}
			if (ans[0] == 'l')
			{
				for (int x = 0; x < 10; x++)
				{
					if (clist[x] > 0)
					{
						//printf("%d..%d", x, clist[x]);
						char newline[5000];
						snprintf(newline, sizeof(newline), "%d -- (ID)%d", x, clist[x]);
						send(clientsocket, newline, strlen(newline) + 1, 0);
						sleep(1);
					}
				}
			}
			if (ans[0] == 'd')
			{
				//dm someone
				char getdm[5000];
				snprintf(getdm, sizeof(getdm), "Enter message to send privately");
				send(clientsocket, getdm, strlen(getdm) + 1, 0);
				char dm[5000];
				recv(clientsocket, dm, 5000, 0);

				for (int i = 0; i < 10; i++)
				{
					if (clist[i] == (int)ans[1] - 48)
					{
						send((int)clist[i], dm, strlen(dm) + 1, 0);
					}
				}
			}
			if (ans[0] == 'r')
			{
				char rmsg[4];
				snprintf(rmsg, sizeof(rmsg), "Quit");

				for (int i = 0; i < 10; i++)
				{
					if (clist[i] == (int)ans[1] - 48)
					{
						//char extra[5000];
						//close(clist[i]);
						send((int)clist[i], rmsg, strlen(rmsg) + 1, 0);
						//recv(clist[i], extra, 5000, 0);
						shutdown((int)clist[i], SHUT_RDWR);
						close((int)clist[i]);
						clist[i] = -20;
						kList[i][0] = 0;
						return 0;
					}
				}
			}
		}
		else
		{
			printf("\n<%d> says: %s", clientsocket, line);
			for(int x = 0; x < 10; x++) {
				if(clist[x]==clientsocket) {
					for(int y = 0; y < 32; y++) {
						dKey[y]=kList[x][y];
					}
				}
			}
			char newline[5000];
			char eline[5000];
			snprintf(newline, sizeof(newline), "<%d> posts: %s", clientsocket, line);
			encrypt(newline, strlen((char*)line), dKey, 0, eline);
			for (int i = 0; i < size; i++)
			{
				if (clist[i] > 0)
				{
					int m = send((int)clist[i], newline, strlen(newline) + 1, 0);
					//printf("sent to %d", clist[i]);
					//printf("message sent to %d", clientsocket);
				}
			}
		}
		//recv and send back out?
	}
	close(clientsocket);
}

int main(int argc, char **argv)
{

	FILE *fp = fopen("RSApriv.pem", "rb");
	PrivateKey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);

	for (int x = 0; x < 10; x++)
	{
		clist[x] = -20;
	};

	//creating socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serveraddr, clientaddr;
	serveraddr.sin_family = AF_INET;

	//reading in port number
	int portnum;
	printf("Enter port number: ");
	scanf("%d", &portnum);
	//printf("\nListening...\n");

	serveraddr.sin_port = htons(portnum);
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	listen(sockfd, 10);

	int len = sizeof(clientaddr);
	//int clientsocket = accept(sockfd, (struct sockaddr*)&clientaddr, &len);

	while (1)
	{
		int clientsocket = accept(sockfd, (struct sockaddr *)&clientaddr, &len);
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
