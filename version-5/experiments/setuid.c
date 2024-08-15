#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[])
{
   if(argc != 3)
      return 1;

   int pid;
   struct stat pre_stat, post_stat;

   uid_t uid = atoi(argv[1]);
   gid_t gid = atoi(argv[2]);
   
   if(fork() == 0) {
   int pre = creat("/tmp/tg-suid-demo-pre", 00777);
   stat("/tmp/tg-suid-demo-pre",  &pre_stat);
   unlink("/tmp/tg-suid-demo-pre");

   printf("Before setuid: euid=%d, uid=%d\n", geteuid(), getuid());
   printf("setuid errno %d return: %d errno = %d\n", errno, seteuid(uid), errno);
   printf(" After setuid: euid=%d, uid=%d\n\n", geteuid(), getuid());

   printf("Before setgid: egid=%d, gid=%d\n", getegid(), getgid());
   printf("setgid errno %d return: %d\n", errno, setegid(gid));
   printf(" After setgid: egid=%d, gid=%d\n\n", getegid(), getgid());

   int post = creat("/tmp/tg-suid-demo-post", 00777);
   stat("/tmp/tg-suid-demo-post", &post_stat);
   unlink("/tmp/tg-suid-demo-post");

   printf(" File owner pre setting uid/gid: u=%d g=%d\n",
      pre_stat.st_uid, pre_stat.st_gid);

   printf("File owner post setting uid/gid: u=%d g=%d\n",
      post_stat.st_uid, post_stat.st_gid);
   }
   else {
	   wait(NULL);
   printf("parent setuid: euid=%d, uid=%d\n", geteuid(), getuid());
   }

   return 0;

}
