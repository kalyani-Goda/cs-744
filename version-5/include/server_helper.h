/*
 * This file declares the functions defined by server helper files.
 * Developed as part of DECS Lab/Project.
 *
 * Author: Santhosh
 * Copyright: CSE@IITB
 *
 */


#include <sys/stat.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <sqlite3.h>
#include "constants.h"
#include "utils.h"
#include "sqldb.h"

/*
 * Structure for queue entries.
 */
struct entry {
	int token;
	int uid;
	sqlite3 *db;
	double starttime;
	TAILQ_ENTRY (entry) entries;
};

/*
 * Structure for queue entries.
 */
struct status_entry {
	int socketfd;
	sqlite3 *db;
	int uid;
	double starttime;
	TAILQ_ENTRY (status_entry) entries;
};

struct file_request {
	int token;
	int socketfd;
	int uid;
	sqlite3 *db;
};

struct params {
	long cpu;
	long mem;
	int threads;
	long fsize;
	int fno;
	int nusers;
	char *chroot;
	long *uids;
};

extern double request_completed, error_count;
extern pthread_mutex_t global_lock;
/*
 * This function sets CPU affinity of the thread specified.
 * Return: returns 0 on success otherwise errno is returned.
 */
int set_affinity(pthread_t thread, int cpu_affinity);

/*
 * This function take the filename of program, filename to store errors and 
 * filename of the output (executable file) name.
 * Return: returns the value of the system command (0 on success).
 */
int compile(int uid, char *filename, char *error_file, char *compiled_file);

/*
 * This function take the executable filename of program, filename to store 
 * errors and filename to store the output of the program execution.
 * Return: returns the value of the system command (0 on success).
 */
int execute(int uid, char *filename, char *error_file, char *output_file);

/*
 * This function take the output file and expected result file and stores the 
 * difference in diff_file name.
 * Return: returns the value of the system command (0 on no difference).
 */
int compare(int uid, char *filename, char *expected_file, char *diff_file);


void getCompileFileName(int requestId, char *dir, char *filename, int n);
void getErrorFileName(int requestId, char *dir, char *filename, int n);
void getExecutableFileName(int requestId, char *dir, char *filename, int n);
void getOutputFileName(int requestId, char *dir, char *filename, int n);
void getDiffFileName(int requestId, char *dir, char *filename, int n);

void queue_initialize();
int add_to_queue(sqlite3 *db, int token, int uid, double starttime);
int add_to_status_queue(sqlite3 *db, int socketfd, int uid, double starttime);

void *autograder(void *thread_arg);

void *file_receiver_thread(void *args);

void *statuschecker(void *thread_arg);

int getParams(char *filename, struct params *params);

int mkdir_p(const char *path, int mode);

void getBaseDirName(int uid, char *dirname, int n);

int uid_send_recev_file(int uid, int requestId, int socketfd, int sendFlag);

int set_compile(int uid, int requestId);

int exec_command(int uid, char *cmd, char **argv, char *error_file, char *output_file);

void createDirectories(int uid);
