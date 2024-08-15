/*
 * This file is the server thread pool implementation.
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */
#define _GNU_SOURCE
#include "server_helper.h"

//Queue to add the commands for the tasks
TAILQ_HEAD(tailhead, entry) head;
TAILQ_HEAD(statustailhead, status_entry) statushead;

// Mutex and Conditional variable for task to wait for command to be issued.
pthread_cond_t cond_queue = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_queue = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_status_queue = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_status_queue = PTHREAD_MUTEX_INITIALIZER;

// Initialize thread queue
void queue_initialize()
{
    TAILQ_INIT(&head);
    TAILQ_INIT(&statushead);
}

// Function to add to queue
int add_to_queue(sqlite3 *db, int token, double starttime) {
    struct entry *elem;
    elem = malloc(sizeof(struct entry));
    if(elem) {
        elem->token = token;
        elem->db = db;
        elem->starttime = starttime;
    }
    else {
	 return -1;
    }

    pthread_mutex_lock(&lock_queue);
    TAILQ_INSERT_TAIL(&head, elem, entries);
    pthread_cond_signal(&cond_queue);
    pthread_mutex_unlock(&lock_queue);
    return 0;
}

// Function to add to status queue
int add_to_status_queue(sqlite3 *db, int socketfd, double starttime) {
    struct status_entry *elem;
    elem = malloc(sizeof(struct status_entry));
    if(elem) {
        elem->socketfd = socketfd;
        elem->db = db;
        elem->starttime = starttime;
    }
    else {
	return -1;
    }

    pthread_mutex_lock(&lock_status_queue);
    TAILQ_INSERT_TAIL(&statushead, elem, entries);
    pthread_cond_signal(&cond_status_queue);
    pthread_mutex_unlock(&lock_status_queue);
    return 0;
}

// Function to handle the client request
int autoresult(sqlite3 *db, int requestId) {
    char filename[LOCAL_BUFF_SIZE];
    char error_file[LOCAL_BUFF_SIZE];
    char diff_file[LOCAL_BUFF_SIZE];
    char output_file[LOCAL_BUFF_SIZE];
    char execute_file[LOCAL_BUFF_SIZE];

    getCompileFileName(requestId, filename);
    getErrorFileName(requestId, error_file);
    getExecutableFileName(requestId, execute_file);
    getOutputFileName(requestId, output_file);
    getDiffFileName(requestId, diff_file);

    char *msg = "P";
    int err = compile(filename, error_file, execute_file);
    if(err != 0) {
        msg = "C";
    }
    else if( (err = execute(execute_file, error_file, output_file) != 0)) {
	printf("execute error = %d\n",err);
	if(err == 124)
		msg = "T";
	else
        	msg = "E";
    }
    else if(compare(output_file, "output/expected.txt", diff_file) != 0) {
        msg = "O";
    }

    err = sqldb_updateStatus(db, requestId, DB_RESULT_STATUS, msg);
    if(err != 0) {
			printf("Not able to update result status for %d (%s) \n", requestId, filename);
	}
	else {
			err = sqldb_updateStatus(db, requestId, DB_REQUEST_STATUS, "C");
			if(err != 0) {
				printf("Not able to update request status for %d (%s) \n", requestId, filename);
			}
	}

	if(err == 0) {
			remove(filename);
			remove(execute_file);
			if(msg[0] == 'P') {
				remove(error_file);
				remove(output_file);
				remove(diff_file);
			}
			else if (msg[0] == 'O') {
				remove(error_file);
				remove(output_file);
			}
			else if ((msg[0] == 'E') || (msg[0] == 'T')) {
				remove(error_file);
			}
	}
    return err;
}

// Function executed by the threads in the thread pool
void *autograder(void *thread_arg) {
    
    //Thread ID was passed by main thread as an argument,
    int id = pthread_self();
    struct entry *elem;
    int requestId;
	sqlite3 *db;
    double starttime, endtime;
        
    while (1) { // loop forever, waiting for counting task

        // Wait for command to be issued by main thread.
        pthread_mutex_lock(&lock_queue);
        while(TAILQ_EMPTY(&head)) {
            pthread_cond_wait(&cond_queue, &lock_queue);
        }

        elem = TAILQ_FIRST(&head);
        TAILQ_REMOVE(&head, elem, entries);
        pthread_mutex_unlock(&lock_queue);

        requestId = elem->token;
        db = elem->db;
        starttime = elem->starttime;
        free(elem);

        // request for the autograding
        int err = autoresult(db, requestId);
        endtime = GetTime();
	sqldb_updateTime(db, requestId, starttime, endtime);
     }
     
     pthread_exit(NULL);
     return NULL;
}

// Function to handle the client request
int checkstatus(sqlite3 *db, int sockfd) {
    // receive request id
    int requestid = -1;
    int err = recv_msg(sockfd, (char *)&requestid, sizeof(requestid));
    if (err < 0) {
        printf("Error receiving request id\n");
        close(sockfd);
	return -1;
    }

    char result[2];
    err = sqldb_getStatus(db, requestid, DB_REQUEST_STATUS, result);
    if(err != 0) {
	    printf("Unable to get status\n");
	    close(sockfd);
	    return -1;
    }

    int status = sqldb_request_status_int(result);
    err = send_request(sockfd, status);
    if (err < 0) {
        printf("Error sending request id\n");
        close(sockfd);
	return -1;
    }

    /* if not done, exit */
    if(status != REQUEST_COMPLETED) {
	close(sockfd);
	return 0;
    }

    err = sqldb_getStatus(db, requestid, DB_RESULT_STATUS, result);
    if(err != 0) {
	    printf("Unable to get result\n");
	    close(sockfd);
	    return -1;
    }

    status = sqldb_result_status_int(result);
    char *msg = UNKNOWN_ERROR_MSG;
    char *file_to_send = NULL;
    char filename[LOCAL_BUFF_SIZE];
    char error_file[LOCAL_BUFF_SIZE];
    char diff_file[LOCAL_BUFF_SIZE];
    char output_file[LOCAL_BUFF_SIZE];
    char execute_file[LOCAL_BUFF_SIZE];

    getCompileFileName(requestid, filename);
    getErrorFileName(requestid, error_file);
    getExecutableFileName(requestid, execute_file);
    getOutputFileName(requestid, output_file);
    getDiffFileName(requestid, diff_file);


    switch(status) {
	    case RESULT_PASS:
        		msg = PASS_MSG;
			break;
	    case RESULT_COMPILE:
			msg = COMPILER_ERROR_MSG;
			file_to_send = error_file;
			break;
	    case RESULT_ERROR:
			msg = RUNTIME_ERROR_MSG;
			file_to_send = error_file;
			break;
	    case RESULT_OUTPUT_ERROR:
			msg = OUTPUT_ERROR_MSG;
			file_to_send = diff_file;
			break;
	    case RESULT_TIMEOUT:
			msg = TIMEOUT_ERROR_MSG;
			file_to_send = NULL;
			break;
	    default:
			msg = UNKNOWN_ERROR_MSG;
			file_to_send = NULL;
			break;
    }

    err = send_msg(sockfd, msg, sizeof(msg));
    if(file_to_send != NULL)
        err = send_file(sockfd, file_to_send);

    close(sockfd);
    return err;
}

// Function executed by the threads in the thread pool
void *statuschecker(void *thread_arg) {
    
    //Thread ID was passed by main thread as an argument,
    int id = pthread_self();
    struct status_entry *elem;
    int sockfd;
    sqlite3 *db;
    double starttime, endtime;
        
    while (1) { // loop forever, waiting for counting task

        // Wait for command to be issued by main thread.
        pthread_mutex_lock(&lock_status_queue);
        while(TAILQ_EMPTY(&statushead)) {
            pthread_cond_wait(&cond_status_queue, &lock_status_queue);
        }

        elem = TAILQ_FIRST(&statushead);
        TAILQ_REMOVE(&statushead, elem, entries);
        pthread_mutex_unlock(&lock_status_queue);

        sockfd = elem->socketfd;
        db = elem->db;
        starttime = elem->starttime;
        free(elem);

        // request for the autograding
        int err = checkstatus(db, sockfd);
    	pthread_mutex_lock(&global_lock);
    	request_completed++;
    	pthread_mutex_unlock(&global_lock);
        endtime = GetTime();
     }
     
     pthread_exit(NULL);
     return NULL;
}
