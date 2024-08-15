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
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>


extern struct params gparam;

void updatelimits(struct config *profile) {
	profile->cpu = gparam.cpu;
	profile->memory = gparam.mem;
	profile->fsize = gparam.fsize;
	profile->nproc = gparam.threads;

}

void mySetLimits (struct config profile)
{
	struct rlimit rlim;

       getrlimit(RLIMIT_CPU, &rlim);
       rlim.rlim_cur = profile.cpu;
       setrlimit(RLIMIT_CPU, &rlim);

       getrlimit(RLIMIT_AS, &rlim);
       rlim.rlim_cur = profile.cpu;
       setrlimit(RLIMIT_AS, &rlim);

       getrlimit(RLIMIT_FSIZE, &rlim);
       rlim.rlim_cur = profile.cpu;
       setrlimit(RLIMIT_FSIZE, &rlim);

       getrlimit(RLIMIT_NPROC, &rlim);
       rlim.rlim_cur = profile.cpu;
       setrlimit(RLIMIT_NPROC, &rlim);

}

void changeRoot() {
	    if(chdir(gparam.chroot) != 0) {
		printf("Unable to change dir %s errno = %d, uid = %d\n", gparam.chroot, errno, getuid());
		perror("Unable to chdir directory");

	    }
	    if(chroot(gparam.chroot) != 0) {
		perror("Unable to chroot directory");
	    }
}

// Function to handle the client request
int uid_send_recev_file(int uid, int requestId, int socketfd, int sendFlag)
{
    char dirname[LOCAL_BUFF_SIZE];
    char filename[LOCAL_BUFF_SIZE];
    pid_t cpid, w;
    int wstatus;
	int err;
	struct config safeprofile = { 1, 32768, 0, 0, 8192, 8192, 0, 10, 5000, 65535 };

    cpid = fork();
    if(cpid == 0) {
		/* Fork and safe exec with uids */
		changeRoot();
	printf("child before pid = %d\n", getuid());
	    int olduid = getuid();

	    int localerr = setuid(uid);
	    if(localerr != 0) {
		    printf("Unable to set uid (%d) \n", uid);
	    }
	printf("child after pid = %d\n", getuid());

	    /* Update profile with file size limit */
	//	updatelimits(&safeprofile);
	 //   mySetLimits(safeprofile);

	    /* now set the limits */
		if (sendFlag == 0) {
				printf("Inital directory creation with user = %d\n", getuid());
				createDirectories(uid);

				getBaseDirName(uid, dirname, sizeof(dirname));
				getErrorFileName(requestId, dirname, filename, sizeof(filename));
				getExecutableFileName(requestId, dirname, filename, sizeof(filename));
				getOutputFileName(requestId, dirname, filename, sizeof(filename));
				getDiffFileName(requestId, dirname, filename, sizeof(filename));
				getCompileFileName(requestId, dirname, filename, sizeof(filename));

				// Get the source file
				err = recv_file(socketfd, filename);
		}
	    /* now set the limits */
		else {
				getBaseDirName(uid, dirname, sizeof(dirname));
				getCompileFileName(requestId, dirname, filename, sizeof(filename));
				printf("filename = %s\n", filename);
				err = send_file(socketfd, filename);
		}
	    setuid(olduid);
	    exit(err);
    }
    else {
	    /* Wait for child process */
	    do {
                   w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
                   if (w == -1) {
                       perror("waitpid");
                       exit(EXIT_FAILURE);
                   }

                   if (WIFEXITED(wstatus)) {
                       printf("exited, status=%d\n", WEXITSTATUS(wstatus));
                   } else if (WIFSIGNALED(wstatus)) {
                       printf("killed by signal %d\n", WTERMSIG(wstatus));
					   err = WTERMSIG(wstatus);
					   break;
                   } else if (WIFSTOPPED(wstatus)) {
                       printf("stopped by signal %d\n", WSTOPSIG(wstatus));
					   err = WTERMSIG(wstatus);
					   break;
                   } else if (WIFCONTINUED(wstatus)) {
                       printf("continued\n");
                   }
		       err = WEXITSTATUS(wstatus);
               } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));


	    if(err < 0) {
		printf("Error reading content from soket");
	    }
    }
    return err;
}

// Function to handle the client request
int set_compile(int uid, int requestId)
{
    char dirname[LOCAL_BUFF_SIZE];
    char filename[LOCAL_BUFF_SIZE];
    char error_file[LOCAL_BUFF_SIZE];
    char diff_file[LOCAL_BUFF_SIZE];
    char output_file[LOCAL_BUFF_SIZE];
    char execute_file[LOCAL_BUFF_SIZE];
    pid_t cpid, w;
    int wstatus;
	int err = 0;
	int msg = 'P';
	struct config safeprofile = { 1, 32768, 0, 0, 8192, 8192, 0, 10, 5000, 65535 };

    cpid = fork();
    if(cpid == 0) {
		/* Fork and safe exec with uids */
		changeRoot();

	    /* Update profile with file size limit */
		updatelimits(&safeprofile);
	    mySetLimits(safeprofile);

		getBaseDirName(uid, dirname, sizeof(dirname));
	    getErrorFileName(requestId, dirname, error_file, sizeof(error_file));
	    getExecutableFileName(requestId, dirname, execute_file, sizeof(execute_file));
	    getOutputFileName(requestId, dirname, output_file, sizeof(output_file));
	    getDiffFileName(requestId, dirname, diff_file, sizeof(diff_file));
	    getCompileFileName(requestId, dirname, filename, sizeof(filename));

		err = compile(uid, filename, error_file, execute_file);
		if(err != 0) {
			msg = 'C';
		}
		else if( (err = execute(uid, execute_file, error_file, output_file) != 0)) {
			printf("execute error = %d\n",err);
			if(err == 124)
				msg = 'T';
			else
				msg = 'E';
		}
		else if((err = compare(uid, output_file, "/output/expected.txt", diff_file)) != 0) {
			msg = 'O';
		}

		printf("message after execution is %d\n", msg);
	    exit(msg);
    }
    else {
	    /* Wait for child process */
	    do {
                   w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
                   if (w == -1) {
                       perror("waitpid");
                       exit(EXIT_FAILURE);
                   }

                   if (WIFEXITED(wstatus)) {
                       printf("exited, status=%d\n", WEXITSTATUS(wstatus));
                   } else if (WIFSIGNALED(wstatus)) {
                       printf("killed by signal %d\n", WTERMSIG(wstatus));
					   err = WTERMSIG(wstatus);
					   break;
                   } else if (WIFSTOPPED(wstatus)) {
                       printf("stopped by signal %d\n", WSTOPSIG(wstatus));
					   err = WTERMSIG(wstatus);
					   break;
                   } else if (WIFCONTINUED(wstatus)) {
                       printf("continued\n");
                   }
		       err = WEXITSTATUS(wstatus);
               } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));


	    if(err < 0) {
		printf("Error reading content from soket");
	    }
    }
	printf("2 status %d\n", err);
    return err;
}

// Function to handle the client request
int exec_command(int uid, char *cmd, char **argv, char *error_file, char *output_file)
{
    pid_t cpid, w;
    int wstatus;
	int err;
	struct config safeprofile = { 1, 32768, 0, 0, 8192, 8192, 0, 10, 5000, 65535 };

    cpid = fork();
    if(cpid == 0) {
	    /* Update profile with file size limit */
		updatelimits(&safeprofile);
	    mySetLimits(safeprofile);

		int olduid = getuid();
	    int localerr = setuid(uid);
	    if(localerr != 0) {
		    printf("Unable to set uid (%d) \n", uid);
	    }

		if(error_file != NULL) {
	    	fclose(stderr);
			int fd = open(error_file, O_WRONLY | O_CREAT);
			if(fd < 0) {
					printf(" not able to open error file\n");
			}
            chmod (error_file, 0640);
		}

		if(output_file != NULL) {
	    	fclose(stdout);
			int fd = open(output_file, O_WRONLY | O_CREAT);
			if(fd < 0) {
					printf(" not able to open error file\n");
			}
            chmod (output_file, 0640);
		}
		printf("%s cmd \n", cmd);
		int i = 0;
		do {
				printf("index %d arg %s\n", i, argv[i]);
		}
		while(argv[++i] != NULL);
        err = execv (cmd, argv); //, NULL);
		//printf("execl %s arg = %s pwd = %s err = %d errno = %d\n", cmd, argv[0], get_current_dir_name(), err, errno);
	    exit(err);
    }
    else {
	    /* Wait for child process */
	    do {
                   w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
                   if (w == -1) {
                       perror("waitpid");
                       exit(EXIT_FAILURE);
                   }

                   if (WIFEXITED(wstatus)) {
                       printf("exited, status=%d\n", WEXITSTATUS(wstatus));
                   } else if (WIFSIGNALED(wstatus)) {
                       printf("killed by signal %d\n", WTERMSIG(wstatus));
					   err = WTERMSIG(wstatus);
					   break;
                   } else if (WIFSTOPPED(wstatus)) {
                       printf("stopped by signal %d\n", WSTOPSIG(wstatus));
					   err = WTERMSIG(wstatus);
					   break;
                   } else if (WIFCONTINUED(wstatus)) {
                       printf("continued\n");
                   }
		       err = WEXITSTATUS(wstatus);
			   printf("error status = %d\n", err);
               } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));


	    if(err < 0) {
		printf("Error reading content from soket");
	    }
    }
    return err;
}

