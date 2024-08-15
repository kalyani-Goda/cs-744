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
int add_to_queue(sqlite3 *db, int token, int uid, double starttime) {
    struct entry *elem;
    elem = calloc(sizeof(struct entry), 1);
    if(elem) {
        elem->token = token;
        elem->db = db;
        elem->uid = uid;
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
int add_to_status_queue(sqlite3 *db, int socketfd, int uid, double starttime) {
    struct status_entry *elem;
    elem = calloc(sizeof(struct status_entry), 1);
    if(elem) {
        elem->socketfd = socketfd;
        elem->db = db;
        elem->uid = uid;
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

#define ARGC	30
#define ARGSIZE 4000
extern struct params gparam;

int safeexec_main (int argc, char **argv, char **envp);

// Function to handle the client request
int autoresult(sqlite3 *db, int requestId, int uid) {

	printf("initiating execution \n");

	printf("before safeexec pid = %d\n", geteuid());
	int olduid = geteuid();
    char msg1 = set_compile(uid, requestId);
	printf("after safeexec pid = %d\n", geteuid());
	//seteuid(olduid);
	printf("after complete uid = %d old id = %d\n", geteuid(), olduid);

	char msg[2];

	bzero(msg, sizeof(msg));
	msg[0] = msg1;
    int err = sqldb_updateStatus(db, requestId, DB_RESULT_STATUS, msg);
    if(err != 0) {
			printf("Not able to update result status for %d \n", requestId );
	}
	else {
			err = sqldb_updateStatus(db, requestId, DB_REQUEST_STATUS, "C");
			if(err != 0) {
				printf("Not able to update request status for %d \n", requestId);
			}
	}

    return err;
}

// Function executed by the threads in the thread pool
void *autograder(void *thread_arg) {
    
    //Thread ID was passed by main thread as an argument,
    int id = pthread_self();
    struct entry *elem;
    int requestId, uid;
	sqlite3 *db;
    double starttime, endtime;
    char *loc, *argv[ARGC];

    loc = (char *)calloc(ARGC, ARGSIZE);
    for (int i = 0; i < ARGC; i++) {
	    argv[i] = loc;
	    loc += ARGSIZE;
    }
        
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
        uid = elem->uid;
        starttime = elem->starttime;
        free(elem);

        // request for the autograding
        int err = autoresult(db, requestId, uid);
        endtime = GetTime();
		sqldb_updateTime(db, requestId, starttime, endtime);
     }
     
     pthread_exit(NULL);
     return NULL;
}

// Function to handle the client request
int checkstatus(sqlite3 *db, int sockfd, int uid) {
    // receive request id
    int requestid = -1;
    int err = recv_msg(sockfd, (char *)&requestid, sizeof(requestid));
    if (err < 0) {
        printf("Error receiving request id\n");
        close(sockfd);
	return -1;
    }

	if (sqldb_checkUid(db, requestid, uid) == false) {
	    printf("uids mismatch\n");
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
    err = send_request(sockfd, status, uid);
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

	printf("Checking status %d\n", status);
    getCompileFileName(requestid, gparam.chroot, filename, sizeof(filename));
    getErrorFileName(requestid, gparam.chroot, error_file, sizeof(error_file));
    getExecutableFileName(requestid, gparam.chroot, execute_file, sizeof(execute_file));
    getOutputFileName(requestid, gparam.chroot, output_file, sizeof(output_file));
    getDiffFileName(requestid, gparam.chroot, diff_file, sizeof(diff_file));


    switch(status) {
	    case RESULT_PASS:
        	msg = PASS_MSG;
			file_to_send = NULL;
			break;
	    case RESULT_COMPILE:
			msg = COMPILER_ERROR_MSG;
			file_to_send = error_file;
			filename;
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

	printf("file message = %s\n", msg);
    err = send_msg(sockfd, msg, sizeof(msg));
    if(file_to_send != NULL) {
			printf("before mail pid = %d\n", geteuid());
			err = uid_send_recev_file(uid, requestid, sockfd, 1);
			printf("after mail pid = %d\n", geteuid());
	}

    close(sockfd);
    return err;
}

// Function executed by the threads in the thread pool
void *statuschecker(void *thread_arg) {
    
    //Thread ID was passed by main thread as an argument,
    int id = pthread_self();
    struct status_entry *elem;
    int sockfd, uid;
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
		uid = elem->uid;
        starttime = elem->starttime;
        free(elem);

        // request for the autograding
        int err = checkstatus(db, sockfd, uid);
    	pthread_mutex_lock(&global_lock);
    	request_completed++;
    	pthread_mutex_unlock(&global_lock);
        endtime = GetTime();
     }
     
     pthread_exit(NULL);
     return NULL;
}
