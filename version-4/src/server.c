/*
 * This file declares the functions defined by utils files.
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */
// Include necessary headers

#define _GNU_SOURCE
#include <signal.h>
#include <assert.h>
#include "server_helper.h"

#ifdef SINGLE
#define MAX_THREAD 1
#else
#define MAX_THREAD 100
#endif

//#define CPU_ID 0
#define MAX_BACKLOG_THRESHOLD	1000

#define TRUE 1

// Mutex and Conditional variable for task to wait for command to be issued.
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;

double request_received, request_completed, error_count;

// Signal handler for the SIGPIPE signal
void sigpipe_handler_socketthreads(int unused) {
    printf("ERROR: socket. Thread exiting\n");
    pthread_mutex_lock(&global_lock);
	error_count++;
	pthread_mutex_unlock(&global_lock);
    pthread_exit(NULL);
}

// Main function
int main(int argc, char* argv[]) {
    int sockfd, //the listen socket descriptor (half-socket)
        newsockfd, //the full socket after the client connection is made
        portno; //port number at which server listens

    socklen_t clilen; //a type of an integer for holding length of the socket address
    char buffer[BUFFER_SIZE]; //buffer for reading and writing the messages
    struct sockaddr_in serv_addr, cli_addr; //structure for holding IP addresses
    int thread_pool_size, status_pool_size;
    int n;
    double starttime, endtime;
	sqlite3 *db;
	int token = 0;

    if (argc < 4) {
        fprintf(stderr, "Usage: Server port_number grader_thread_pool_size status_thread_pool_size\n");
        exit(1);
    }

    int err = sqldb_initialize(&db, &token);
	if(err != 0) {
			printf("not abel to open sqlite database\n");
			exit(-1);
	}
     sigaction(SIGPIPE, &(struct sigaction){sigpipe_handler_socketthreads}, NULL);
    /* create socket */ 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //AF_INET means Address Family of INTERNET. SOCK_STREAM creates TCP socket (as opposed to UDP socket)
    // This is just a holder right now, note no port number either. It needs a 'bind' call

    if (sockfd < 0) {
        error("ERROR opening socket");
        exit(1);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr)); // initialize serv_address bytes to all zeros

    serv_addr.sin_family = AF_INET; // Address Family of INTERNET
    serv_addr.sin_addr.s_addr = INADDR_ANY;  //Any IP address.

    //Port number is the first argument of the server command
    portno = atoi(argv[1]);
    thread_pool_size = atoi(argv[2]);
    status_pool_size = atoi(argv[3]);
    serv_addr.sin_port = htons(portno);  // Need to convert number from host order to network order

    /* bind the socket created earlier to this port number on this machine
    First argument is the socket descriptor, second is the address structure (including port number).
    Third argument is size of the second argument */
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    /* listen for incoming connection requests */
    listen(sockfd, 5); // 2 means 2 connection requests can be in queue.
    //now server is listening for connections   

	queue_initialize();
	sqldb_afterRestart(db);

    pthread_t p[thread_pool_size];
    pthread_t status_thread[status_pool_size];

    for (int i = 0; i < thread_pool_size; i++) {
        int rc = pthread_create(&p[i], NULL, autograder, NULL);
          assert(rc == 0);
    }

    for (int i = 0; i < status_pool_size; i++) {
        int rc = pthread_create(&p[i], NULL, statuschecker, NULL);
          assert(rc == 0);
    }

    while (TRUE) {
        
        clilen = sizeof(cli_addr);  //length of struct sockaddr_in
    
        starttime = GetTime();
        /* accept a new request, now the socket is complete.
         Create a newsockfd for this socket.
          First argument is the 'listen' socket, second is the argument
          in which the client address will be held, third is length of second
        */
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        endtime = GetTime();

        pthread_mutex_lock(&global_lock);
		int backlog = (request_received - request_completed - error_count );
        pthread_mutex_unlock(&global_lock);

		if(backlog > MAX_BACKLOG_THRESHOLD) {
			close(newsockfd);
			continue;
		}
		int request_type = -1;
        if (newsockfd < 0) {
            error("ERROR on accept");
        }
        else {
			int err = recv_request(newsockfd, &request_type);
			if(err != 0) {
					printf("Not able to receive request type\n");
					close(newsockfd);
					continue;
			}
            request_received++;
        }

		if(request_type == USER_NEW_REQUEST) {
				pthread_t pt;
				struct file_request *request = (struct file_request *)malloc(sizeof(struct file_request));
				if(request == NULL) {
						printf("out of memory\n");
						close(newsockfd);
						continue;
				}
				request->db = db;
				request->token = token+1;
				request->socketfd = newsockfd;
				token++;
				int rc = pthread_create(&pt, NULL, file_receiver_thread, request);

				// now forget this thread...
				pthread_detach(pt);
		}
		else if(request_type == USER_STATUS_REQUEST) {
				int rc = add_to_status_queue(db, newsockfd, GetTime());
				if(rc != 0) {
						printf("not able to add to status queue\n");
						close(newsockfd);
						continue;
				}
		}
		else {
				printf("Invalid request type\n");
				close(newsockfd);
		}
    }
    return 0;
}
