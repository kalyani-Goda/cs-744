/*
 * This file is the server helper functions.
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */
#define _GNU_SOURCE   
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

int requestHandler(int sockfd, char *filename, int uid) {
    // send request type
    int err = send_request(sockfd, USER_NEW_REQUEST, uid);
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

int check_request_status(int sockfd, int requestid, int uid) {
    // send request type
    int err = send_request(sockfd, USER_STATUS_REQUEST, uid);
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
    err = recv_request(sockfd, &request_status, &uid);
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
    int portno, uid, requestid=-1, argidx=1;

    if (argc < 6) {
        fprintf(stderr, "usage %s servername serverport uid request_type<new/status> request_id/autograder\n", argv[0]);
        exit(0);
    }

    char *hostname = strdup(argv[argidx++]);
    portno = atoi(argv[argidx++]);

    int sockfd = getsock_connection(hostname, portno);
    if(sockfd < 0) {
	    printf("Not able to connect to server\n");
	    exit(0);
    }

    uid = atoi(argv[argidx++]);

    if(strcmp(argv[argidx], "new") == 0) {

	argidx++;
    	char *filename = strdup(argv[argidx++]);

	requestid = requestHandler(sockfd, filename, uid);
	if(requestid <= 0) {
	    perror("new request fialed\n");
	    exit(EXIT_FAILURE);
	}
	printf("RequestID = %d\n", requestid);

    	free(filename);
    }
    else if(strcmp(argv[argidx], "status") == 0) {
	argidx++;
    	requestid = atoi(argv[argidx++]);
	check_request_status(sockfd, requestid, uid);
    }
    free(hostname);

    exit(0);
}
