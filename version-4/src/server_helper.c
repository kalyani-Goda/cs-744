/*
 * This file is the server helper functions.
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */

#define _GNU_SOURCE
#include "constants.h"
#include <pthread.h>


int set_affinity(pthread_t thread, int cpu_affinity) {
    cpu_set_t cpuset;

    /* Set affinity mask to include CPUs 0 to 7 */

    CPU_ZERO(&cpuset);
    CPU_SET(cpu_affinity, &cpuset);

    int s = pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
    if (s != 0)
        printf("pthread_setaffinity_np failed %d",s);

}

int compile(char *filename, char *error_file, char *compiled_file) {
	char cmd[BUFFER_SIZE];
	bzero(cmd, BUFFER_SIZE);

	sprintf(cmd, "gcc -o %s %s 2> %s",compiled_file, filename, error_file);
     	int err = system(cmd);
	return err;
}

int execute(char *filename, char *error_file, char *output_file) {
	char cmd[BUFFER_SIZE];
	bzero(cmd, BUFFER_SIZE);

	/* 10 minutes time out */
	sprintf(cmd, "timeout 600 ./%s 2> %s > %s\n",filename, error_file, output_file);
    int err = system(cmd);
	return err;
}

int compare(char *filename, char *expected_file, char *diff_file) {
	char cmd[BUFFER_SIZE];
	bzero(cmd, BUFFER_SIZE);

	sprintf(cmd, "diff %s %s > %s\n",filename, expected_file, diff_file);
    int err = system(cmd);
	return err;
}

void getCompileFileName(int requestId, char *filename) {
    system("mkdir -p grader/compile 2> /dev/null");

    sprintf(filename, "grader/compile/%d.c", requestId);
}

void getErrorFileName(int requestId, char *filename) {
    system("mkdir -p grader/error 2> /dev/null");
    sprintf(filename, "grader/error/%d.err", requestId);
}

void getExecutableFileName(int requestId, char *filename) {
    system("mkdir -p grader/compile grader/error grader/output 2> /dev/null");
    sprintf(filename, "grader/compile/%d.out", requestId);
}

void getOutputFileName(int requestId, char *filename) {
    system("mkdir -p grader/output 2> /dev/null");
    sprintf(filename, "grader/output/%d.txt", requestId);
}

void getDiffFileName(int requestId, char *filename) {
    sprintf(filename, "grader/output/%d.diff", requestId);
}

    // Get the source file
