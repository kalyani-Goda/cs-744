/*
 * This file is the server helper functions.
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */

#define _GNU_SOURCE
#include "server_helper.h"
#include <pthread.h>

// Function to handle the client request
void *file_receiver_thread(void *args)
{
    struct file_request *request = (struct file_request *)args;
    int requestId = request->token;
    int socketfd = request->socketfd;
    sqlite3 *db = request->db;
    char filename[LOCAL_BUFF_SIZE];

    free(request);
    request = NULL;

    getCompileFileName(requestId, filename);

    // Get the source file
    long err = (long)recv_file(socketfd, filename);
    if(err < 0) {
        printf("Error reading content from soket");
	close(socketfd);
        pthread_mutex_lock(&global_lock);
	error_count++;
        pthread_mutex_unlock(&global_lock);
	pthread_exit((void *)err);
        return (void *)err;
    }
    err = sqldb_insertRequest(db, requestId, filename);
    if(err < 0) {
        printf("Error inserting recordi in DB ");
        remove(filename);
	close(socketfd);
        pthread_mutex_lock(&global_lock);
	error_count++;
        pthread_mutex_unlock(&global_lock);
	pthread_exit((void *)err);
        return (void *)err;
    }

    /* Send Request to Thread Pool */
    add_to_queue(db, requestId, GetTime());

    char msg[MAX_FILE_SIZE_BYTES];
    memcpy(msg, &requestId, sizeof(requestId));
    err = send_msg(socketfd, msg, sizeof(requestId));
    if(err < 0) {
        printf("Error sending fileId to client ");
	close(socketfd);
        pthread_mutex_lock(&global_lock);
	error_count++;
        pthread_mutex_unlock(&global_lock);
	pthread_exit((void *)err);
        return (void *)err;
    }
    close(socketfd);
    pthread_mutex_lock(&global_lock);
    request_completed++;
    pthread_mutex_unlock(&global_lock);
    pthread_exit(NULL);
    return NULL;
}
