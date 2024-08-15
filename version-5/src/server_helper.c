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
#include <sys/time.h>
#include <fcntl.h>
#include <sys/resource.h>


int set_affinity(pthread_t thread, int cpu_affinity) {
    cpu_set_t cpuset;

    /* Set affinity mask to include CPUs 0 to 7 */

    CPU_ZERO(&cpuset);
    CPU_SET(cpu_affinity, &cpuset);

    int s = pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
    if (s != 0)
        printf("pthread_setaffinity_np failed %d",s);

}

int compile(int uid, char *filename, char *error_file, char *compiled_file) {
	char *args[] = {"/usr/bin/gcc", "-o", compiled_file, filename, NULL, NULL};

	printf("compile %s %s\n", compiled_file, filename);
	int err = exec_command(uid, "/usr/bin/gcc", args, error_file, NULL);
	return err;
}

int execute(int uid, char *filename, char *error_file, char *output_file) {
	char *args[] = {filename, NULL};

	printf("execute %s %s\n", filename, output_file);
	int err = exec_command(uid, filename, args, error_file, output_file);
	return err;
}

int compare(int uid, char *filename, char *expected_file, char *diff_file) {
	char *args[] = {"/usr/bin/diff", expected_file, filename, NULL};

	printf("compare %s %s\n", filename, diff_file);
	int err = exec_command(uid, "/usr/bin/diff", args, NULL, diff_file);
	printf("compare returning %d\n", err);
	return err;
}

void getCompileFileName(int requestId, char *dir, char *filename, int n) {
	bzero(filename, n*sizeof(char));
    snprintf(filename, n*sizeof(char), "%s/compile/%d.c", dir, requestId);
}

void getErrorFileName(int requestId, char *dir, char *filename, int n) {
	bzero(filename, n*sizeof(char));
    snprintf(filename, n*sizeof(char), "%s/error/%d.err", dir, requestId);
}

void getExecutableFileName(int requestId, char *dir, char *filename, int n) {
	bzero(filename, n*sizeof(char));
    snprintf(filename, n*sizeof(char), "%s/output/%d.out", dir, requestId);
}

void getOutputFileName(int requestId, char *dir, char *filename, int n) {
	bzero(filename, n*sizeof(char));
    snprintf(filename, n*sizeof(char), "%s/output/%d.txt", dir, requestId);
}

void getDiffFileName(int requestId, char *dir, char *filename, int n) {
	bzero(filename, n*sizeof(char));
    snprintf(filename, n*sizeof(char), "%s/output/%d.diff",dir, requestId);
}

void getBaseDirName(int uid, char *dirname, int n) {
	bzero(dirname, n*sizeof(char));
    snprintf(dirname, n*sizeof(char), "/home/%d", uid);
}

void createDirectories(int uid) {
    char buff[BUFFER_SIZE];
    char dir[BUFFER_SIZE];

	getBaseDirName(uid, dir, sizeof(dir));
	bzero(buff, sizeof(buff));
	snprintf(buff, sizeof(buff), "%s/compile",dir);
    int err = mkdir_p(buff, 0755);
    if(err != 0) {
	    printf("unable to created directory %s\n", buff);
	    perror("Unable to create compile directory");
    }

	bzero(buff, sizeof(buff));
	snprintf(buff, sizeof(buff), "%s/error",dir);
    err = mkdir_p(buff, 0755);
    if(err != 0) {
	    perror("Unable to create error directory");
    }

	bzero(buff, sizeof(buff));
	snprintf(buff, sizeof(buff), "%s/output",dir);
    err = mkdir_p(buff, 0755);
    if(err != 0) {
	    perror("Unable to create output directory");
    }
}

int readline(int fd, char *buffer, int size) {
		// return error if no new line has been found as well
		int  index = 0;
		while( read(fd, &buffer[index], 1) > 0) {
				if(buffer[index] == '\n')
						return 0;
				index++;
		}
		return -1;
}

int getParams(char *filename, struct params *params) {
		int fd = open(filename, O_RDONLY);
		char buffer[BUFFER_SIZE];
		if(fd < 0) {
				printf("Error opening config file %s\n", filename);
				return -1;
		}

		/* Get default limits */
		struct rlimit old;
		// CPU
		int err = getrlimit(RLIMIT_CPU, &old);
		if(err < 0) {
				printf("Error getting CPU limit\n");
				return -1;
		}
		params->cpu = old.rlim_cur;

		err = getrlimit(RLIMIT_DATA, &old);
		if(err < 0) {
				printf("Error getting DATA limit\n");
				return -1;
		}
		params->mem = old.rlim_cur;

		err = getrlimit(RLIMIT_FSIZE, &old);
		if(err < 0) {
				printf("Error getting FSIZE limit\n");
				return -1;
		}
		params->fsize = old.rlim_cur;

		err = getrlimit(RLIMIT_NOFILE, &old);
		if(err < 0) {
				printf("Error getting NOFILE limit\n");
				return -1;
		}
		params->fno = old.rlim_cur;

		err = getrlimit(RLIMIT_NPROC, &old);
		if(err < 0) {
				printf("Error getting thread limit\n");
				return -1;
		}
		params->threads = old.rlim_cur;

		params->nusers = 0;
		params->uids = NULL;
		params->chroot = "/tmp/autograder";

		// Read the config file
		bzero(buffer, sizeof(buffer));
		while(readline(fd, buffer, sizeof(buffer)) == 0) {
			// Check for comment line.
			char *str = trim(buffer);
			if(str[0] == '#')
					continue;

			char *sep = strchr(str, ':');
			if(sep == NULL) {
					printf("Wrong config file\n");
					close(fd);
					return -1;
			}
			
			*sep = ' ';
			char *eol = strchr(str, '#');
			if(eol != NULL)
					*eol = '\0'; 

			if(strstr(str, "cpu") != NULL)
					params->cpu = atoi(sep);
			else if(strstr(str, "mem") != NULL)
					params->mem = atol(sep);
			else if(strstr(str, "fsize") != NULL)
					params->fsize = atol(sep);
			else if(strstr(str, "fno") != NULL)
					params->fno = atoi(sep);
			else if(strstr(str, "chroot") != NULL) {
					char *chroot = (char *)calloc(strlen(sep) , sizeof(char));
					if(chroot == NULL) {
						printf("Error allocating memory for chroot\n");
						close(fd);
						return -1;
					}
					sep= trim(sep);
					memcpy(chroot, sep, strlen(sep));
					params->chroot = chroot;
			}
			else if(strstr(str, "nusers") != NULL)
					params->nusers = atoi(sep);
			else if(strstr(str, "uids") != NULL) {
					if((params->nusers == 0) || (params->nusers > MAX_USERS)) {
							printf("wrong nusers configuration\n");
							close(fd);
							return -1;
					}
					params->uids = (long *)malloc(sizeof(long) * params->nusers);
					if(params->uids == NULL) {
							printf("not able to allocate memory for nusers %d\n", params->nusers);
							close(fd);
							return -1;
					}
					for (int i = 0; i< params->nusers; i++) {
						char *usr = strchr(sep,',');
						if((usr == NULL) && (strlen(sep) < 1)) {
								printf("missing userid\n");
								close(fd);
								return -1;
						}
						if(usr)
						  *usr = '\0';
						params->uids[i] = atol(sep);
						sep = usr + 1;

					}
			}
			bzero(buffer, sizeof(buffer));
		}

		close(fd);

		// Sanity checks of params
		if((params->nusers == 0) || (params->uids == NULL)) {
				printf("wrong nuser\n");
				return -1;
		}

		return 0;
}

/* Make a directory; already existing dir okay */
static int maybe_mkdir(const char* path, mode_t mode)
{
    struct stat st;
    errno = 0;

    /* Try to make the directory */
    if (mkdir(path, mode) == 0)
        return 0;

    /* If it fails for any reason but EEXIST, fail */
    if (errno != EEXIST)
        return -1;

    /* Check if the existing path is a directory */
    if (stat(path, &st) != 0)
        return -1;

    /* If not, fail with ENOTDIR */
    if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return -1;
    }

    errno = 0;
    return 0;
}

int mkdir_p(const char *path, int mode)
{
    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    char *_path = NULL;
    char *p;
    int result = -1;

    errno = 0;

    /* Copy string so it's mutable */
    _path = strdup(path);
    if (_path == NULL)
        goto out;

    /* Iterate the string */
    for (p = _path + 1; *p; p++) {
        if (*p == '/') {
            /* Temporarily truncate */
            *p = '\0';

            if (maybe_mkdir(_path, mode) != 0)
                goto out;

            *p = '/';
        }
    }

    if (maybe_mkdir(_path, mode) != 0) 
        goto out;

    result = 0;

out:
    free(_path);
    return result;
}
