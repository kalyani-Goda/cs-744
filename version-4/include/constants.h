/*
 * This file defines the common constants shared between client and server
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/time.h>

#define LOCAL_BUFF_SIZE	250

#define BUFFER_SIZE  		1024
#define MAX_FILE_SIZE_BYTES  	4
#define MAX_TRIES  		5

#define COMPILER_ERROR_MSG "COMPILER ERROR\n"
#define RUNTIME_ERROR_MSG "RUNTIME ERROR\n"
#define OUTPUT_ERROR_MSG "OUTPUT ERROR\n"
#define TIMEOUT_ERROR_MSG "RUNTIME TIMEOUT ERROR\n"
#define UNKNOWN_ERROR_MSG "UNKNOWN ERROR\n"
#define PASS_MSG "PASS\n"

#define DB_REQUEST_STATUS	10
#define DB_RESULT_STATUS	11

#define USER_NEW_REQUEST	1
#define USER_STATUS_REQUEST	2


#ifndef RequestStatus 
#define RequestStatus
enum RequestStatus {
	REQUEST_COMPLETED=10,
	REQUEST_NEW=20,
	REQUEST_WIP=30,
	REQUEST_UNKNOWN=0
};
#endif

#ifndef ResultStatus
#define ResultStatus
enum ResultStatus {
	RESULT_PENDING=10,
	RESULT_PASS=20,
	RESULT_COMPILE=30,
	RESULT_ERROR=40,
	RESULT_OUTPUT_ERROR=50,
	RESULT_TIMEOUT=60,
	RESULT_UNKNOWN=0
};

#endif
