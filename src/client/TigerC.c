
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include "../../include/processing.h"
#include "../../include/ftpTiger.h"

/*
 * Add any socket variables and other variables you deem necessary
 */
int sock;
int server_port = 8888;
int opt_val = 1;
struct sockaddr_in server_address;
//ftp vector
ftp_msg_t payload;
ftp_put_msg_t file_packet;

char trans_buffer[TRANS_BUFF_SIZE];
char buffer[BUFF_SIZE];
char filename_buff[PATH_LENGTH + 1];
int err;
char delimiter[2] = "\r\0";

int data_len, file_len;
int c_read;

FILE *file;

// utility functions

void sig_handler(int signo){
	if( signo == SIGINT ) {
		close(sock);
		exit(1);
	}
}

// sending stuff 
void tcp_send(char *message, uint16_t type, uint16_t len){
	payload.type = type;
	payload.length = len;
	
	memcpy(trans_buffer, &payload, (size_t)HEADER_SIZE);
	memcpy(trans_buffer + HEADER_SIZE, message, len);
	
	//sending the username and password
	write(sock, trans_buffer, HEADER_SIZE + len);
}

// receiving stuff
int tcp_receive(char *buffer){
	int rd;
	rd = recv(sock, trans_buffer, TRANS_BUFF_SIZE, 0);
	
	// handle the data, assuming the there's no buffer overflow
	// if there's is oops let's just trash the rest of the content for now
	memcpy( &payload, trans_buffer, (size_t)HEADER_SIZE);
	if(payload.type == T_DISCONNECT) {
		fprintf(stderr, "Server got shut down for some reason. \n\n");
		return -1;
	}
	else{
		memcpy(buffer, trans_buffer+HEADER_SIZE, payload.length);
		return payload.length;
	}

} 

/*
 * Fill in code for the Login process here. This function is called by the
 * lexer library wheen the user insantiaties the TCONNECT command. Be sure
 * to return a success status of 0 or failure status of -1. This informs
 * the lexer code of the authentication status. Note that this only accounts
 * for the authentication of the client side. Be sure you support an
 * authentication with the server side as well.
 * 
 * @param uint32_t  IPv4 Address of the server as a 32-bit number
 * @param char *    NULL-terminated Usersname char array
 * @param char *    NULL-terminated Password char array
 * 
 * @returns int     Success status: 0 success, -1 failure
 */
int ProcessLogin( uint32_t sipaddr, char *username, char *password ) {
  printf("Library called the login function <%s> <%s>\n", username, password);
  // Place your connection and authentication process code here
  
	signal(SIGINT, sig_handler);

	//allocate memory, initialize server addr
	memset(&server_address, 0, sizeof(server_address));	
	server_address.sin_family = AF_INET;	
	server_address.sin_addr.s_addr = htonl(sipaddr);
	server_address.sin_port = htons(server_port);
	
	//create the endpoint socket
	if((sock = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)) < 0){
		fprintf(stderr, "Could not create socket.\n");
		return -1;
	}
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);
	
	memset(&buffer, 0, sizeof(buffer));
	memset(&trans_buffer, 0, sizeof(trans_buffer));
	
	err = connect(sock, (struct sockaddr *) &server_address,\
				sizeof(server_address));
	if(err != 0 ){
		fprintf(stderr, "Error trying to connect to the server. Crying.\n");
	}
	
	memset(&payload, 0, sizeof(ftp_msg_t));
	
	//preparing to send
	data_len = strlen(username);
	strncpy(buffer, username, data_len);
	strncpy(buffer + data_len, delimiter, 1);
	data_len += 1;
	data_len += strlen(password);
	strncpy(buffer + data_len, password, strlen(password));
	tcp_send(buffer, FTP_CNT, (uint16_t)data_len);
	
	c_read = tcp_receive(buffer);
	if(c_read == -1){
		fprintf(stderr, "Error: Could nor hear from the server. Bye.\n");
		close(sock);
		return -1;
	}
	else{
		if(payload.type != CNT_SUCCESS){
			fprintf(stderr, "tconnect: invalid credentials. \n");
			close(sock);
			return -1;
		}
	}
	
  Authenticated();
  return 0;
}

/*
 * This function is called by the lexer code when the user submits the exit 
 * command after the client is already logged in and authenticated with the 
 * server.  This function should close the connection with the server and 
 * then call the Deauthenticate() function.
 * 
 * @return int      Success status: 0 success, -1 failure
 */
int ProcessLogout() {
  // Place your disconnection and memory deallocation code here
  	payload.type = T_DISCONNECT;
	payload.length = (uint16_t) 0;
	memcpy(trans_buffer, &payload, (size_t)HEADER_SIZE);
	write(sock, trans_buffer, strlen(trans_buffer));
	close(sock);
  
  Deauthenticate();
}

/*
 * Fill in code for the TPUT command. This function accepts a NULL-terminated
 * character array containing the filename of the local filesystem file to 
 * push to the server. You must open and read the local file, and then send the 
 * file to the serveer along with the filename of the file. Return the status 
 * of the operation as an integer value (using POSIX standards).
 * 
 * @param char*     Filename of the local file to send
 * 
 * @returns int     Status of the operation 0 for success, -1 for failure
 */
int ProcessPutFile( char *filename ) {
  printf("Library called the TPUT function with file: <%s>\n", filename );
  // Place your File Upload code here
  file = fopen(filename, "r");
  if(file == NULL){
  	fprintf(stderr, "tput: Cannot open the file: %s\n", filename);
  	return -1;
  }
  data_len = strlen(filename);
  memcpy( (buffer + HEADER_SIZE), filename, (size_t)data_len);
  // maxbuff: header + filename + content
  c_read = fread((buffer + HEADER_SIZE + data_len), sizeof(char),\
  		 (BUFF_SIZE - HEADER_SIZE - data_len), file);
  		 
  file_packet.filename_len = data_len;
  file_packet.file_len = c_read;
  memcpy(buffer, &file_packet, (size_t)HEADER_SIZE);
  
  if(c_read < (BUFF_SIZE - HEADER_SIZE - data_len)){
  	tcp_send(buffer, FTP_PUT, (uint16_t) (data_len + c_read + HEADER_SIZE));
  }
  
  //not done with the file yet.
  else{
  	tcp_send(buffer, FTP_PUT, (uint16_t) BUFF_SIZE);
  	do{
  		c_read = fread(buffer, sizeof(char), BUFF_SIZE, file);
  		tcp_send(buffer, FTP_PUT2, (uint16_t) c_read);
  	} while (c_read >= BUFF_SIZE);
  	// reached end of file
  }
  
  fclose(file);
  return 0;
}

/*
 * Fill in code for the TGET command. This function accepts a NULL-terminated
 * character array containing the filename of the server filesystem file to 
 * pull from the server. You must send the command and filename to the server.
 * The server should report a success/failure status to the clieent, and if 
 * success, it should then transmit the entire file.  This funciton must then
 * store the data using the same filename on the local filesystem.
 * 
 * @param char*     Filename of the remote file to receive
 * 
 * @returns int     Status of the operation 0 for success, -1 for failure
 */
int ProcessGetFile( char *filename ) {
  printf("Library called the TGET function with file: <%s>\n", filename );
  // Place your File Download code here
  tcp_send(filename, FTP_GET, (uint16_t) strlen(filename));
  
  c_read = tcp_receive(buffer);
  if(payload.type == FTP_FAIL){
  	fprintf(stderr, "tget: File %s cannot be accessed. \n\n", filename);
  	return -1;
  }
  else if(payload.type != FTP_PUT ){
  	fprintf(stderr, "tget: Unknown type of server response.\n\n");
  	return -1;
  }
  
  //start parsing the files
  memcpy(&file_packet, buffer, HEADER_SIZE);
  data_len = file_packet.filename_len;
  file_len = file_packet.file_len;
  memcpy(filename_buff, buffer + HEADER_SIZE, data_len);
  
  file = fopen(filename_buff, "w");
  if(file == NULL){
  	fprintf(stderr, "tget: could not open a file to write. crying.\n\n");
  	return -1;
  }
  fwrite((buffer + HEADER_SIZE + data_len), sizeof(char), file_len, file);
  while (payload.length >= BUFF_SIZE){
  		c_read = tcp_receive(buffer);
  		if(payload.type != FTP_PUT2){
  			fprintf(stderr, "tget: interrupted during receiving the file. \n\n");
  			fclose(file);
  			return -1;
  		}
  		fwrite(buffer, sizeof(char), c_read, file);
  }
  //has read everything from the file.
  fclose(file);

  return 0;
}

/*
 * This function should send a command to the server to get a listing of the 
 * server's current working directory, for the thread or context serving the 
 * client instance. This implies that the server should be able to have different
 * working directories active for each authenticated client. The server should 
 * return success or failure, and if success, it should alos provide the 
 * listing of the server's currwnt directory.
 * 
 * @returns int     Status of the operation 0 for success, -1 for failure
 */
int ProcessListDir() {
  printf("Library called the TLIST function\n");
  // Place your Directory Listing code here
  	payload.type = FTP_LST;
	payload.length = (uint16_t) 0;
	memcpy(trans_buffer, &payload, (size_t)HEADER_SIZE);
	write(sock, trans_buffer, strlen(trans_buffer));
	
	// get reply
	c_read = tcp_receive(buffer);
	if(payload.type == FTP_LST){
		buffer[payload.length] = '\0';
		printf("%s\n\n", buffer);
	}
	else{
		fprintf(stderr, "tlist: unknown error.\n\n");
		return -1;
	}

  return 0;
}

/*
 * This function is used to change the working directory of the server for the 
 * active client only. Changing the server's working directory from one instance
 * of the client should have no affect on another active instance of the same 
 * client. The server should report success or failure.
 * 
 * @returns int     Status of the operation 0 for success, -1 for failure
 */
int ProcessChangeDir( char *dirname ) {
  printf("Library called the TCWND function: <%s>\n", dirname);
  // Place your Change Directory request code here
	tcp_send(dirname, FTP_CWD, (uint16_t) strlen(dirname));
  return 0;
}

/*
 * This function is used to perform an emergency shutdown. If the STDIN stream
 * returns an EOF, then the client must close the active connection and clean
 * up any dyanmic memory or files opened. When you return from this function
 * the ProcessCommand loop will break out and return to the main function to 
 * complete the app processing.
 *
 */
void EmergencyShutdown() {
	payload.type = T_DISCONNECT;
	payload.length = (uint16_t) 0;
	memcpy(trans_buffer, &payload, (size_t)HEADER_SIZE);
	write(sock, trans_buffer, strlen(trans_buffer));
	close(sock);
}


/*****************************************************************************
 * Main Entry - Add any statup code required
 *****************************************************************************/
int main( int argc, char **argv ) 
{
  /*
   * Place any startup code you need here
   */
  	//file message buffer
  	memset(&file_packet, 0, sizeof(ftp_put_msg_t));
  
  
  
  /*
   * This is the call to the command line parser and tokenizer. You should not
   * need to modify anything within the library or the parser.  If you do, you 
   * will need to readup on the re2c 1.0.1 program.
   */
  ProceessLoop();
  return 0;
}
