/*
 * This file is the server thread pool implementation.
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */
#include <assert.h>
#include <stdlib.h>
#include "utils.h"

void error(char *msg) {
  perror(msg);
  exit(0);
}

double GetTime() {
    struct timeval t;
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);
    return (double) t.tv_sec + (double) t.tv_usec/1e6;
}

int send_file(int sockfd, char * file_path) {
	char buffer[BUFFER_SIZE];
	bzero(buffer, BUFFER_SIZE);

	/* Check for args */
	if(file_path == NULL) {
		printf("file path is NULL\n");
		return -1;
	}
	if(sockfd < 0) {
		printf("Sockfd is wrong\n");
		return -1;
	}
	FILE *file = fopen(file_path, "rb");
	if(!file)
	{
                printf("Failed to open file %s\n", file_path);
		perror("Error opening file \n");
		return -1;
	}
	fseek(file, 0, SEEK_END);
	int file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char file_size_bytes[MAX_FILE_SIZE_BYTES];
	memcpy(file_size_bytes, &file_size, sizeof(file_size));
	if(send(sockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1)
	{
		perror("Error sending file size\n");
		fclose(file);
		return -1;
	}

	while(!feof(file)) {
		size_t bytes_read = fread(buffer, sizeof(char), sizeof(buffer), file);
		if(send(sockfd, buffer, bytes_read, 0) == -1)
		{
			perror("Error sending file data\n");
			fclose(file);
			return -1;
		}
		bzero(buffer, BUFFER_SIZE);
	}
	fclose(file);
	return 0;
}

int recv_file(int sockfd, char * file_path) {
	char buffer[BUFFER_SIZE];
	bzero(buffer, BUFFER_SIZE);

	FILE *file = NULL;

	if(sockfd < 0) {
		printf("Sockfd is wrong\n");
		return -1;
	}

	if(file_path == NULL) 
		file = stdout;
	else
		file = fopen(file_path, "wb");
	if(!file)
	{
		perror("Error opening file\n");
		return -1;
	}
	
	char file_size_bytes[MAX_FILE_SIZE_BYTES];
	if(recv(sockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1)
	{
		perror("Error receiving file size\n");
		fclose(file);
		return -1;
	}
	int file_size;
	memcpy(&file_size, file_size_bytes, sizeof(file_size_bytes));

    int bytes_read = 0;
	while(bytes_read < file_size) {
		int recv_bytes = recv(sockfd, buffer, BUFFER_SIZE, 0);
		if(recv_bytes < 0) {
			perror("error receiveing file data\n");
			fclose(file);
			remove(file_path);
			return -1;
		}
		fwrite(buffer, sizeof(char), recv_bytes, file);
		bzero(buffer, BUFFER_SIZE);
		bytes_read += recv_bytes;
	}
        if(file_path)
	    fclose(file);
	return 0;
}

int recv_msg(int sockfd, char *msg_buffer, int msg_buffer_size) {
	
	if(sockfd < 0) {
		printf("Sockfd is wrong\n");
		return -1;
	}
	char file_size_bytes[MAX_FILE_SIZE_BYTES];
	if(recv(sockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1)
	{
		perror("Error receiving file size\n");
		return -1;
	}
	int file_size;
	memcpy(&file_size, file_size_bytes, sizeof(file_size_bytes));

	/* Check for overflow */
	int bytes_left = file_size - msg_buffer_size;
	if(file_size > msg_buffer_size)
		file_size = msg_buffer_size;

	int recv_bytes = recv(sockfd, msg_buffer, file_size, 0);
	if(recv_bytes < 0) {
			perror("error receiveing msg data\n");
			return -1;
	}

	/* Flush out the data */
	if(bytes_left > 0) {
	    char buffer[BUFFER_SIZE];
	    recv_bytes = recv(sockfd, buffer, bytes_left, 0);
		perror("dropping extra buffer\n");

	}
	return recv_bytes;
}

int send_msg(int sockfd, char *msg_buffer, int msg_buffer_size) {
	
	if(sockfd < 0) {
		printf("Sockfd is wrong\n");
		return -1;
	}
	char file_size_bytes[MAX_FILE_SIZE_BYTES];
	memcpy(file_size_bytes, &msg_buffer_size, sizeof(file_size_bytes));
	if(send(sockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1)
	{
		perror("Error sending msg size\n");
		return -1;
	}

	int recv_bytes = send(sockfd, msg_buffer, msg_buffer_size, 0);
	if(recv_bytes < 0) {
			perror("error sending msg data\n");
			return -1;
	}
	return recv_bytes;
}

int recv_request(int sockfd, int *request_type) {
	
	if(sockfd < 0) {
		printf("Sockfd is wrong\n");
		return -1;
	}
	char file_size_bytes[MAX_FILE_SIZE_BYTES];
	if(recv(sockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1)
	{
		perror("Error receiving file size\n");
		return -1;
	}
	memcpy(request_type, file_size_bytes, sizeof(file_size_bytes));

	return 0;
}

int send_request(int sockfd, int request_type) {
	
	if(sockfd < 0) {
		printf("Sockfd is wrong\n");
		return -1;
	}
	char file_size_bytes[MAX_FILE_SIZE_BYTES];
	memcpy(file_size_bytes, &request_type, sizeof(file_size_bytes));
	if(send(sockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1)
	{
		perror("Error sending msg size\n");
		return -1;
	}

	return 0;
}
