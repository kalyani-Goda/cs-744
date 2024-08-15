/*
 * This file declares the functions defined by utils files.
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */

#define _GNU_SOURCE   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include "utils.h"

int getsock_connection(char *hostname, int portno) {
    int sockfd;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
	return -1;
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
	return sockfd;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        if (errno != EINPROGRESS) {
            perror("ERROR connecting");
            close(sockfd);
	    return -1;
        } 
    }

    return sockfd;
}

int requestHandler(int sockfd, char *filename) {
    // send request type
    int err = send_request(sockfd, USER_NEW_REQUEST);
    if (err < 0) {
        printf("Error sending request type\n");
        close(sockfd);
	return -1;
    }

    // Send file
    err = send_file(sockfd, filename);
    if (err < 0) {
        printf("Error sending file\n");
        close(sockfd);
	return -1;
    }
    

    // receive request id
    int requestid = -1;
    err = recv_msg(sockfd, (char *)&requestid, sizeof(requestid));
    if (err < 0) {
        printf("Error receiving request id\n");
        close(sockfd);
	return -1;
    }
    return requestid;
}

int check_request_status(int sockfd, int requestid) {
    // send request type
    int err = send_request(sockfd, USER_STATUS_REQUEST);
    if (err < 0) {
        printf("Error sending request type\n");
        close(sockfd);
	return -1;
    }
    // send request id
    err = send_msg(sockfd, (char *)&requestid, sizeof(requestid));
    if (err < 0) {
        printf("Error sending request id\n");
        close(sockfd);
	return -1;
    }
    // receive request status
    int request_status = -1;
    err = recv_request(sockfd, &request_status);
    if (err < 0) {
        printf("Error receving request status\n");
        close(sockfd);
	return -1;
    }

    if(request_status != (int)REQUEST_COMPLETED) {
        printf("request not completed\n");
        close(sockfd);
	return 0;
    }
    char result[BUFFER_SIZE];
    bzero(result, BUFFER_SIZE);
    err = recv_msg(sockfd, result, BUFFER_SIZE);
    if (err < 0) {
        printf("Error receiving result message\n");
        close(sockfd);
        return -1;
    }

    printf("Server result: %s\n", result);

    if (strcmp(result, "PASS\n") != 0) {
    
        err = recv_file(sockfd, NULL);
        if (err < 0) {
            printf("Error receiving result message\n");
            close(sockfd);
	    return -1;
        }
    }
    close(sockfd);
    return 0;
}


int main(int argc, char *argv[]) {
    int portno, requestid=-1;

    if (argc < 5) {
        fprintf(stderr, "usage %s servername serverport request_type<new/status> request_id/autograder\n", argv[0]);
        exit(0);
    }

    char *hostname = strdup(argv[1]);
    portno = atoi(argv[2]);

    int sockfd = getsock_connection(hostname, portno);
    if(sockfd < 0) {
	    printf("Not able to connect to server\n");
	    exit(0);
    }

    if(strcmp(argv[3], "new") == 0) {

    	char *filename = strdup(argv[4]);

	requestid = requestHandler(sockfd, filename);
	if(requestid <= 0) {
	    perror("new request fialed\n");
	    exit(EXIT_FAILURE);
	}
	printf("RequestID = %d\n", requestid);

    	free(filename);
    }
    else if(strcmp(argv[3], "status") == 0) {
    	requestid = atoi(argv[4]);
	check_request_status(sockfd, requestid);
    }
    free(hostname);

    exit(0);
}
