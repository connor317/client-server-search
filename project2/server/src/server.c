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

static void handleErrors()
{
  ERR_print_errors_fp(stderr);
  abort();
}

static int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
  unsigned char *iv, unsigned char *ciphertext)
{
  EVP_CIPHER_CTX *ctx;

  int len;

  int ciphertext_len;

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
  if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)){
    handleErrors();
  }
  ciphertext_len = len;

  /* Finalise the encryption. Further ciphertext bytes may be written at
   * this stage.
   */
  if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)){
    handleErrors();
  }
  ciphertext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return ciphertext_len;
}

static int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
  unsigned char *iv, unsigned char *plaintext)
{
  EVP_CIPHER_CTX *ctx;

  int len;

  int plaintext_len;

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

  return plaintext_len;
}

/*
	this function calculates the length of a string
	@param string this is the char * that is wanted the length to be calculated
	@return the lenth of the string
*/
static int length(char* string){
	int len = 0;
	while(*(string+len) != '\0'){
		len++;
	}
	return len;
}

/*
	This function check to see if any invalid characters are in the string that is passed to this function
	@param name this is the string that is being tested
	return_int is returned and tells if the string is valid or not
	good_chars is incremented everytime a character passes the test and is compared to the length of the string to see if it is a valid string in the end.  
*/

static int check(char *name){
	int i = 0;
	int return_int = 0;
	int good_chars = 0;

	while (*(name+i) != '\0'){
		if((*(name+i) == '/') || (*(name+i) == '\\') || (*(name+i) == '=') || (*(name+i) == '$') || (*(name+i) == '*') || (*(name+i) == '&')){
			printf("Error: invalid character %c detected at index %d\n", name[i], i);
		}
		else{
			good_chars++;
		}
			
		i++;
	};

	if(good_chars == length(name)){
		return_int = 1;
	}
	else{
		return_int = 0;
	}

	return return_int;
}
/*
	Searches through a file for a string and sends the result back to a client
	Sends error messages if invalid strings are sent.  
	if the string is found then the line number is returned to the client.  
	continues until the end of the file is reached.  
*/

int find(char *temp1, char * temp2, int num_args, int client_connection){
	unsigned char *key = (unsigned char *)"01234567890123456789012345678901";

	/* A 128 bit IV */
	unsigned char *iv = (unsigned char *)"0123456789012345";
	unsigned char *encrypt_send_str=calloc(128, sizeof(char));
	char *word = calloc(128, sizeof(char));
	char *filename = calloc(128, sizeof(char));
	char *send_str = calloc(128, sizeof(char));	
	
	int count = 0;
	int line_num = 1;
	int num_found = -1;
	FILE * iFile;
	int word_length = 0;
	char character;
	
	strncpy(filename, temp1, 127);
	
	
	if(check(filename) == 0){
		exit(0);
	}
	
	
	strncpy(word, temp2, 127);
	word_length = length(word);
	if(check(word) == 0){
		exit(0);
	}	
	
	
	iFile = fopen(filename, "r");
	if(iFile == NULL){
		printf("Error getting file\n");
	}
	else if(num_args != 3){
		(void)snprintf(send_str, 127, "Incorrect format\n");
		(void)encrypt((unsigned char *)send_str, length((char *)send_str), key, iv, encrypt_send_str);
		(void)send(client_connection, encrypt_send_str, 128, 0);
	
		(void)snprintf(send_str, 127, "Should look like:\n./build/file");
		(void)encrypt((unsigned char *)send_str, length((char *)send_str), key, iv, encrypt_send_str);
		(void)send(client_connection, encrypt_send_str, 128, 0);
		
		(void)snprintf(send_str, 127, "<word to search for>\n");
		(void)encrypt((unsigned char *)send_str, length((char *)send_str), key, iv, encrypt_send_str);
		(void)send(client_connection, encrypt_send_str, 128, 0);	
	}
	else{
		num_found=0;
		
		character = (char)fgetc(iFile);
		do{
			if (character == '\n'){
				line_num += 1;
								
			}
			else if (character == *(word+count)){
				if (count == (word_length-1)){
					
					count = 0;
					(void)snprintf(send_str, 127, "Found on line: %d\n", line_num);
					
					(void)encrypt((unsigned char *)send_str, length((char *)send_str), key, iv, encrypt_send_str);
					
					(void)send(client_connection, encrypt_send_str, 128, 0);
					
					num_found++;
					(void)sleep(1);					
				}						
				else{
					count += 1;			
				}
				
			}
			else if( character == *(word+0)){
				count = 1;
			}
			else{
				count = 0;
			}
			character = (char)fgetc(iFile);
		}while((int)character != EOF);
		(void)fclose(iFile);
		if(num_found == 0){
			
			(void)snprintf(send_str, 127, "The String is not in file\n");
		}
		else{
			
			(void)snprintf(send_str, 127, "There were %d found\n", num_found);
	
		}
	}
	
	(void)encrypt((unsigned char *)send_str, strnlen(send_str, 127), key, iv, encrypt_send_str);
	

	(void)write(client_connection, encrypt_send_str, 128);
	
	free(filename);
	free(word);	
	free(encrypt_send_str);
	free(send_str);
	return num_found;
}

/*
	searches the license.txt file that is recieved from the client.
	recieves the string information from the client
	decrypts the string that is received from the client
	sends the decrypted string to the search function 
	closes the client connection and waits for another one.  
*/

static void serverSearch(){
	unsigned char *key = (unsigned char *)"01234567890123456789012345678901";

	/* A 128 bit IV */
	unsigned char *iv = (unsigned char *)"0123456789012345";

	

	/* Buffer for the decrypted text */
	unsigned char *decryptedtext = calloc(128,sizeof(char));
	
	int decryptedtext_len = 0; 
	
	unsigned char *data_packet=calloc(129, sizeof(char));
	unsigned char *incoming_data=calloc(129, sizeof(char));
	
	int count = 0;
	int incoming_client = 0, client_connection = 0;
	unsigned int client_count = 1;
	struct sockaddr_in server_ip;
	incoming_client = socket(AF_INET, SOCK_STREAM, 0); //socket
	memset(&server_ip, 0, sizeof(server_ip));
	memset(data_packet, 0, 129);
	server_ip.sin_family = AF_INET;
	server_ip.sin_addr.s_addr = htonl(INADDR_ANY);
	server_ip.sin_port = htons(2018);
	(void)bind(incoming_client, (struct sockaddr*)&server_ip, (socklen_t)sizeof(server_ip));
	(void)listen(incoming_client, 20);

	while(1)
	{
		printf("\n\nServer Listening for Client %u\n", client_count);
		client_connection = accept(incoming_client, (struct sockaddr*)NULL, NULL);
		printf("Client recieved\n");

		(void)read(client_connection, incoming_data, 128);

		decryptedtext_len = decrypt(incoming_data, 16, key, iv, decryptedtext);
		decryptedtext[decryptedtext_len] = '\0';
		
		printf("Searching in file: license.txt\n");
		printf("Looking for string: %s\n", (char *)decryptedtext);
		
		count = find((char *)"license.txt", (char *)decryptedtext, 3, client_connection);
		
		printf("There were %d found\n", count);
		
		(void)close(client_connection);
		client_count++;
		(void)sleep(1);
		memset(incoming_data, 0, 129);
		
	}

}

/*
	The program starts here. 
*/
int main(){
	serverSearch();
	return 0;
}

