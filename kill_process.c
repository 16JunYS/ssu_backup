#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "ssu_backup.h" 
int pid;

void kill_handler(int signo) { //handler function on SIGUSR
	int fd;
	time_t t;
	struct tm *lt;
	char *message = malloc(50);

	t = time(NULL);
	lt = localtime(&t);
	sprintf(message, "[%02d%02d %02d:%02d:%02d] ssu_backup<pid:%d> exit\n", lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, pid+1);
	fd = open("./backup/backup_log.txt", O_RDWR| O_CREAT | O_APPEND, 0644);
	write(fd, message, strlen(message));	
	close(fd);
	raise(SIGKILL);
} 
void fnd_proc() { //see if ssu_backup daemon process is running
	DIR *dir;
	struct dirent **namelist;
	struct stat statbuf;
	int count, i;
	int fd, offset;
	int proc, temp;
	char *pid_str = malloc(8);
	char proc_path[20];
	char *message = malloc(20);

	if (signal(SIGUSR1, kill_handler) == SIG_ERR) { //call kill_handler() if signal SIGUSR1 is received
		fprintf(stderr, "cannot handle SIGUSR1\n");
		exit(EXIT_FAILURE);
	}
	if ((count = scandir("/proc", &namelist, NULL, alphasort)) == -1) { //scan directory proc if there is existing ssu_backup daemon process
		fprintf(stderr, "/proc directory scan error \n");
		exit(1);
	}
	for (i = 0, proc = 0; i < count; i++) {
		if(strcmp("self", namelist[i]->d_name) == 0) {
			continue;
		}
		sprintf(proc_path, "/proc/%s", namelist[i]->d_name);	
		stat(proc_path, &statbuf);
		if (S_ISDIR(statbuf.st_mode) == 0) {
			continue;
		}
		sprintf(proc_path, "/proc/%s/stat", namelist[i]->d_name);
		//debug)
		//printf("//%d/: %s\n", getpid(), proc_path);
		offset = strlen(namelist[i]->d_name) + 2;
		
		if ((fd = open(proc_path, O_RDONLY)) < 0) {
			continue;
		}
		else {
			lseek(fd, offset, SEEK_SET);

			read(fd, message, 20);
			if (strncmp("ssu_backup", message, 10) == 0) {
				pid = atoi(namelist[i]->d_name);	
				if (pid < getpid()) {
					kill(pid, SIGUSR1); 
					return;
				}	
			}
			close(fd);
		}
	}
	for(i = 0; i < count; i++)
		free(namelist[i]);
	free(namelist);
}
