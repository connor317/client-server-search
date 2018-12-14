#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "../hdr/client.h"


static void handleErrors()
{
  ERR_print_errors_fp(stderr);
  abort();
}

static int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
  unsigned char *iv, unsigned char *ciphertext)
{
  EVP_CIPHER_CTX *ctx;

  int len = 0;

  int ciphertext_len = 0;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())){
  handleErrors();
  }

  /* Initialise the encryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits */
  if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)){
    handleErrors();
  }

  /* Provide the message to be encrypted, and obtain the encrypted output.
   * EVP_EncryptUpdate can be called multiple times if necessary
   */
  if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    handleErrors();
  ciphertext_len = len;

  /* Finalise the encryption. Further ciphertext bytes may be written at
   * this stage.
   */
  if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
  ciphertext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);
  free(ctx);
  return ciphertext_len;
}

static int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
  unsigned char *iv, unsigned char *plaintext)
{
  EVP_CIPHER_CTX *ctx;

  int len = 0;

  int plaintext_len = 0;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())){
    handleErrors();
  }
  /* Initialise the decryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits */
  if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)){
    handleErrors();
  }
  /* Provide the message to be decrypted, and obtain the plaintext output.
   * EVP_DecryptUpdate can be called multiple times if necessary
   */
  if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)){
    handleErrors();
  }
  plaintext_len = len;

  /* Finalise the decryption. Further plaintext bytes may be written at
   * this stage.
   */
  if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)){
    handleErrors();
  }
  plaintext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);
  //free(ctx);
  return plaintext_len;
}

static int length(char* string){
	int len = 0;
	while(*(string+len) != '\0'){
		len++;
	}
	return len;
}

/*
	Sends information to the server
	recieves the string that needs to be sent over to the server
	uses embeded encryption key and iv to encrypt the string that will be sent over the socket
	creates a connection to the server
	sends the encrypted string to the server and waits for a response
	decrypts the response from the server and prints out the results
	at the end of the function the status of this function is returned so that it can be used for test cases and troubleshooting.

*/

int serverSearch(char *arg1){
	/* A 256 bit key */
	unsigned char *key = (unsigned char *)"01234567890123456789012345678901";

	/* A 128 bit IV */
	unsigned char *iv = (unsigned char *)"0123456789012345";

	ssize_t n=0;
	int code = -1;
	int client_socket = 0;
	
	unsigned char *word = calloc(128, sizeof(char));
	
		
	unsigned char *ciphertext = calloc(129, sizeof(unsigned char));
	
	
	unsigned char *encrypt_word = calloc(128, sizeof(unsigned char));
	
	/* Buffer for the decrypted text */
	unsigned char *decryptedtext = calloc(128, sizeof(unsigned char));
	
	int decryptedtext_len = 0;
	
	
	
	struct sockaddr_in server_ip;
	if(strnlen(arg1, 128) > 15){
		code = -1;
	}
	else{
		strncpy((char *)word, arg1, 127);
		(void)encrypt(word, length((char *)word), key, iv, encrypt_word);
		if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Socket not created \n");
		
		}
		else{
			server_ip.sin_family = AF_INET;
			server_ip.sin_port = htons(2018);
			server_ip.sin_addr.s_addr = inet_addr("127.0.0.1");

			if(connect(client_socket, (struct sockaddr *)&server_ip, (socklen_t)sizeof(server_ip))<0) {
				printf("Connection failed due to port and ip problems\n");
			
			}	
			else{
			
				(void)send(client_socket, encrypt_word, (size_t)length((char *)encrypt_word), 0);
			
			
				while((n = read(client_socket, ciphertext, 128)) > 0)
				{				
					decryptedtext_len = decrypt(ciphertext, 32, key, iv, decryptedtext);
								
					*(decryptedtext+decryptedtext_len) = '\0';
				
					printf("%s\n", (char *)decryptedtext);
				
					memset(ciphertext, 0, 129);
					code++;
				
				}
				if (n < 0 )
				{
					printf("Standard input error \n");
					code = -1;
				}
		
			}//end of else
		}//end of else
	}	
	free(ciphertext);
	free(decryptedtext);
	free(encrypt_word);
	free(word);
	return code;
}

/*
	Start of the program
	Errors out if the arguments are not valid.
*/

#ifndef TEST
int main(int argc, char *argv[])
{
	if( (argc != 2) || (strnlen(*(argv+1), 128) > 15) ){
		printf("Incorrect format. Should be: ./build/file <filename> <string>\n");
		printf("string should not be longer than 15 characters\n");
		exit(0);
	}
	else
	{	
		(void)serverSearch(*(argv+1));
	}
	return 0;
}
#endif
