/*
 * This file declares the functions defined by utils files.
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */

#include "constants.h"

/*
 * This function prints the msg in STDERR and exits the process.
 * return: None as the process would exit.
 */
void error(char *msg);

/*
 * This functions returns the current time in seconds with microsecond 
 * precisions.
 * return: double value
 */
double GetTime();

/*
 * This function sends the data from the file_path to the socket.
 * Logic: First sends MAX_FILE_SIZE_BYTES (4) bytes with file size, 
 * followed by the actual file data.
 * Arguments: Sockfd should be great than or equal to 0
 *            file_path should not be NULL
 * Return: -1 on error and 0 on success.
 */
int send_file(int sockfd, char * file_path);

/*
 * This function receives data from socket and writes to the file_path. 
 * Logic: First expects MAX_FILE_SIZE_BYTES (4) bytes which is file size, 
 * followed by actual file data.
 * Arguments: Sockfd should be great than or equal to 0
 *            file_path = if NULL, it writes to STDOUT
 * Return: -1 on error and 0 on success.
 */
int recv_file(int sockfd, char * file_path);

/*
 * This function receives message from the socket.
 * Logic: First expects MAX_FILE_SIZE_BYTES (4) bytes followed by actual 
 * message.
 * Arguments: Sockfd should be great than or equal to 0
 *            msg_buffer
 *            msg_buffer_size - size of the buffer
 * Return: -1 on error and actual data received on success.
 *         if received message is greater than the msg_buffer_size, 
 *         the message is truncated.
 */
int recv_msg(int sockfd, char *msg_buffer, int msg_buffer_size);

/*
 * This function sends message to the socket.
 * Logic: First sends the size of message followed by actual message.
 * Arguments: Sockfd should be great than or equal to 0
 *            msg_buffer
 *            msg_buffer_size
 * Return: -1 on error and actual data semt on success.
 */
int send_msg(int sockfd, char *msg_buffer, int msg_buffer_size);

int recv_request(int sockfd, int *request_type);
int send_request(int sockfd, int request_type);
