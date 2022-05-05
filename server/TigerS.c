/*
* filename: TigerS.c
* description: the Tiger ftp server that handles 100 clients concurrently :) 
* arthor: Tianran C
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "../include/ftpTiger.h"

// comment out the lines below for a larger receive buffer
//#undef	TRANS_BUFF_SIZE
//#define TRANS_BUFF_SIZE	132

#ifndef	MAX_CLIENTS
#define MAX_CLIENTS	100
#endif

#define on_error(...) {fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }

void *client_handler(void *client_fd_ptr);

pthread_mutex_t	mutex;
pthread_mutex_t filewrite_mutex;
int	num_clients;


int main (int argc, char *argv[]){
	int port = 8888;
	int server_fd, client_fd;
	struct sockaddr_in server, client;
	in_addr_t ip;
	int opt_val = 1;
	char ip_str[16];
	int err;
	socklen_t client_addr_len;

	//threading stuff
	pthread_t thread_id;
	int full;

	//parsing arguments
	if(argc < 2){
		fprintf(stderr, "Please provide a valid IP address to start the server.\n");
		exit(1);
	}

	if( strlen(argv[1]) > 15 ){
		fprintf(stderr, "Please provide a valid IP address.\n");
		exit(1);
	}
	
	memset(&server, 0, sizeof(server));
	
	//parsing ip
        //strncpy(ip_str, argv[1], (strlen(argv[1]) < 15 )? strlen(argv[1]): 15);
        if( inet_pton(AF_INET, argv[1], &(ip)) != 1){
                fprintf(stderr, "Please provide a valid IP address.\n");
                exit(1);
        }
        inet_ntop(AF_INET, &ip, ip_str, 16);
	printf("IP:%s\n", ip_str);

	//initializing server socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) on_error("Could not create server socket.\n");

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = ip;

	//should be able to rerun right away...?
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);
	
	err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
	if (err < 0) on_error("Could not bind socket\n");

	// i guess backlog is using fd 128
	err = listen(server_fd, 128);
	if (err < 0) on_error("Could not listen on socket\n");

	printf("Server is listening on port:%d\n", port);

	
	memset(&client, 0, sizeof(client));
	client_addr_len = sizeof(client);

	memset(&mutex, 0, sizeof(pthread_mutex_t));
	memset(&filewrite_mutex, 0, sizeof(pthread_mutex_t));
	err = pthread_mutex_init(&mutex, NULL);
	err = pthread_mutex_init(&filewrite_mutex, NULL);

	//start listening for connections	
	while(1){
		pthread_mutex_lock(&mutex);
                full = (num_clients >= MAX_CLIENTS)? 1: 0;
                pthread_mutex_unlock(&mutex);
		if(full == 1){
			sleep(10);
			continue;
		}

		client_fd = accept(server_fd, (struct sockaddr *)&client,\
				  &client_addr_len);
		if (client_fd < 0) on_error("Could not establish new connection.\n");
	
		//checking if server is at its max capacity
		pthread_mutex_lock(&mutex);
		full = (num_clients >= MAX_CLIENTS) ? 1: 0;
		pthread_mutex_unlock(&mutex);

		inet_ntop(AF_INET, &(client.sin_addr.s_addr), ip_str, 16);
		printf("Client %d connected, IP: %s\n\n", client_fd, ip_str);

		err = pthread_create(&thread_id, NULL, client_handler,\
				     (void *) &client_fd);

		if (err){
			fprintf(stderr,\
			"ERR: client %d, pthread_create() returned %d\n",\
				client_fd, err);
		}
		pthread_mutex_lock(&mutex);
		num_clients ++;
		pthread_mutex_unlock(&mutex);
		
	}
	return 0;	
	
} 


void *client_handler(void *client_fd_ptr){
	int read = 0;
        ftp_msg_t data;
        ftp_put_msg_t file_data;
        char path[PATH_LENGTH];
        char dir[PATH_LENGTH];
        char line[PATH_LENGTH];
	char buffer[TRANS_BUFF_SIZE];
	int client_fd = (int) (*(int *)client_fd_ptr);
	FILE *file;
	
	int path_length, dir_length;
	
	memset(&data, 0, sizeof(ftp_msg_t));
	memset(&file_data, 0, sizeof(ftp_put_msg_t));	
	path[0] = '\0';
	dir[0] = '\0';
	dir_length = 0;
		
	//connection has been established.
	while(1){
		read = recv(client_fd, buffer, TRANS_BUFF_SIZE, 0);
		//handle the data
                 memcpy( &data, buffer, (size_t)HEADER_SIZE);
                 if(data.type == T_DISCONNECT){
                 	printf("Received Disconnect signal from the client.\n");
                 	break;
                 }
                 
                 switch (data.type) {
                 // connect
                 case FTP_CNT :
                 {
                 	// for now, don't even check, just go. 
                 	data.type = CNT_SUCCESS;
                 	data.length = 0;
                 	memcpy(buffer, &data, (size_t)HEADER_SIZE);
                 	write(client_fd, buffer, HEADER_SIZE);
                 	break;
                 }
                 //
                 case FTP_GET :
                 {
                 	path_length = data.length;
                 	memcpy(path+dir_length, (buffer + HEADER_SIZE), (size_t)path_length);
                 	path[path_length + dir_length] = '\0';
                 	file = fopen(path, "r");
                 	if(file == NULL){
                 		data.type = FTP_FAIL;
                 		data.length = 0;
                 		memcpy(buffer, &data, (size_t)HEADER_SIZE);
                 		write(client_fd, buffer, HEADER_SIZE);
                 		printf("CLient %d: tget %s failed.\n\n", client_fd, path);
                 	}
                 	else{
                 		//set file name in the message to be sent
 				memcpy( (buffer + HEADER_SIZE + HEADER_SIZE),\
 				 	(path + dir_length), (size_t)path_length);
  				// transbuff: header + header + filename + content
  				read = fread((buffer + HEADER_SIZE + HEADER_SIZE + path_length),\
  						 sizeof(char),\
  						 (BUFF_SIZE - HEADER_SIZE - path_length), file);
  				file_data.filename_len = path_length;
  				file_data.file_len = read;
  				memcpy(buffer + HEADER_SIZE, &file_data, (size_t)HEADER_SIZE);
  				data.type = FTP_PUT;
  				data.length = HEADER_SIZE + path_length + read;
  				memcpy(buffer, &data, (size_t)HEADER_SIZE);
  				
  				if(read < (int)(BUFF_SIZE - HEADER_SIZE - path_length)){
  					write(client_fd, buffer, (HEADER_SIZE + data.length));
  				}
  				//not done with the file yet.
  				else{
  					write(client_fd, buffer, TRANS_BUFF_SIZE);
  					do{
  					read = fread(buffer + HEADER_SIZE, sizeof(char),\
  							BUFF_SIZE, file);
  					data.type = FTP_PUT2;
  					data.length = read;
  					memcpy(buffer, &data, (size_t)HEADER_SIZE);
  					write(client_fd, buffer,\
  						(uint16_t) (read + HEADER_SIZE));
  					} while (read >= BUFF_SIZE);
  				// reached end of file
  				} 
  				fclose(file);
  				printf("CLient %d: tget %s success.\n\n", client_fd, path);
                 	}
                 	break;
                 }
                 case FTP_PUT :
                 {
                 	memcpy(&file_data, (buffer + HEADER_SIZE), (size_t)HEADER_SIZE );
                 	path_length = file_data.filename_len;
                 	memcpy((path + dir_length), (buffer + HEADER_SIZE + HEADER_SIZE),\
                 		(size_t)path_length);
                 	path[(path_length + dir_length)] = '\0';
                 	
                 	pthread_mutex_lock(&filewrite_mutex);
                 	file = fopen(path, "w");
                 	if(file == NULL){
                 		//user is not expecting an ack for now.
                 		/**
                 		data.type = FTP_FAIL;
                 		data.legnth = 0;	
                 		memcpy(buffer, &data, (size_t)HEADER_SIZE);
                 		write(client_fd, buffer, HEADER_SIZE);
                 		**/
                 		pthread_mutex_unlock(&filewrite_mutex);
                 		printf("Client %d: tput %s failed.\n\n", client_fd, path);
                 	}
                 	fwrite((buffer + HEADER_SIZE + HEADER_SIZE + path_length), sizeof(char),\
                 		file_data.file_len, file);
                 	while(read >= (int)TRANS_BUFF_SIZE){
                 		read = recv(client_fd, buffer, TRANS_BUFF_SIZE, 0);
                 		memcpy( &data, buffer, (size_t)HEADER_SIZE);
                 		if(data.type != FTP_PUT2 ){
                 			fprintf(stderr, "tput: interrupted during receiving the file. \n\n");
					//user is not expecting an ack for now.
					break;
                 		}
                 		else{
                 			fwrite((buffer + HEADER_SIZE), sizeof(char),\
                 				(read - HEADER_SIZE), file);
                 		}
                 	}
                 	fclose(file);
                 	pthread_mutex_unlock(&filewrite_mutex);
                 	printf("Client %d: tput %s success.\n\n", client_fd, path);
                 	break;
                 }
                 case FTP_LST :
                 {
                 	sprintf(buffer, "ls %s 2>&1", dir);
                 	printf("%s\n", buffer);
                 	file = popen(buffer, "r");
                 	if(file == NULL){
                 		data.type = FTP_FAIL;
                 		data.length = 0;
                 		memcpy(buffer, &data, (size_t)HEADER_SIZE);
                 		write(client_fd, buffer, HEADER_SIZE);
                 		printf("Client %d: tlist failed.\n\n", client_fd);
                 	}
                 	else{
                 		buffer[HEADER_SIZE] = '\0';
                 		while( ! feof(file) ){
                 		if (fgets(line, PATH_LENGTH, file) != NULL){
                 			strcat((buffer + HEADER_SIZE), line);
                 		}
                 		}
                 		pclose(file);
                 		data.type = FTP_LST;
                 		data.length = strlen((buffer + HEADER_SIZE));
                 		memcpy(buffer, &data, (size_t) HEADER_SIZE);
                 		write(client_fd, buffer, (data.length + HEADER_SIZE));
                 		printf("Client %d: tlist success.\n\n", client_fd);
                 	}
                 	break;
                 }
                 case FTP_CWD :
                 {
                 	dir_length = data.length;
                 	memcpy(dir, (buffer + HEADER_SIZE), dir_length);
                 	dir[dir_length] = '/';
                 	dir_length++;
                 	dir[dir_length] = '\0';
                 	memcpy(path, dir, (dir_length+1));
                 	printf("Client %d: tcwd. new path: %s\n\n", client_fd, dir);
                 	break;
                 }
                 
                 default : 
                 {
                 	//uhh just trash it i guess. 
                 	break;
                 }        
			
		}
        }
	close(client_fd);
	pthread_mutex_lock(&mutex);
	num_clients --;
	pthread_mutex_unlock(&mutex);
        printf("Client %d disconnected.\n\n\n", client_fd);
	return NULL;
}



