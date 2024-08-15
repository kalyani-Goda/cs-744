/*
 * This file is the server helper functions.
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */

#define _GNU_SOURCE
#include <sys/resource.h>
#include "server_helper.h"
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "safeexec.h"
#include "setlimits.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

extern struct params gparam;

// Function to handle the client request
void *file_receiver_thread(void *args)
{
    struct file_request *request = (struct file_request *)args;
    int requestId = request->token;
    int socketfd = request->socketfd;
    int uid = request->uid;
    sqlite3 *db = request->db;
    char filename[LOCAL_BUFF_SIZE];
    char dirname[LOCAL_BUFF_SIZE];
    pid_t cpid, w;
    int wstatus;

    free(request);
    request = NULL;

	printf("expected uid = %d\n", uid);
	getBaseDirName(uid, dirname, sizeof(dirname));
	getCompileFileName(requestId, dirname, filename, sizeof(filename));
	int err = uid_send_recev_file(uid, requestId, socketfd, 0);
    if(err < 0) {
        printf("Error reading content from soket");
	close(socketfd);
        pthread_mutex_lock(&global_lock);
	error_count++;
        pthread_mutex_unlock(&global_lock);
	pthread_exit((void *)((long)err));
        return (void *)((long)err);
    }
    err = sqldb_insertRequest(db, requestId, uid, filename);
    if(err < 0) {
        printf("Error inserting recordi in DB ");
        remove(filename);
	close(socketfd);
        pthread_mutex_lock(&global_lock);
	error_count++;
        pthread_mutex_unlock(&global_lock);
	pthread_exit((void *)((long)err));
        return (void *)((long)err);
    }

    /* Send Request to Thread Pool */
    add_to_queue(db, requestId, uid, GetTime());

    char msg[MAX_FILE_SIZE_BYTES];
    memcpy(msg, &requestId, sizeof(requestId));
    err = send_msg(socketfd, msg, sizeof(requestId));
    if(err < 0) {
        printf("Error sending fileId to client ");
	close(socketfd);
        pthread_mutex_lock(&global_lock);
	error_count++;
        pthread_mutex_unlock(&global_lock);
	pthread_exit((void *)((long)err));
        return (void *)((long)err);
    }
    close(socketfd);
    pthread_mutex_lock(&global_lock);
    request_completed++;
    pthread_mutex_unlock(&global_lock);
    pthread_exit(NULL);
    return NULL;
}
