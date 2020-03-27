#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "ssu_backup.h"

extern char* Filenameonly;
char* logfile = "./backup/backup_log.txt";
void write_log(backupInfo info, int state) {
	char message[1024];
	int fd, size;

	struct tm *lt;	
	char b_date[4];
	char b_h[2];
	char b_m[2];
	char b_s[2];

	if((fd = open(logfile, O_RDWR | O_CREAT | O_APPEND, 0644)) < 0) {
		fprintf(stderr, "open %s error\n", logfile);
		exit(1);
	}
	//lseek(fd, 0, SEEK_END);
	
	if ((info.backupt = time(NULL)) < 0) {
		fprintf(stderr, "time() call error\n");
		exit(1);
	}

	if ((lt = localtime(&info.backupt)) == NULL) {
		fprintf(stderr, "localtime() call error\n");
		exit(1);
	}
	sprintf(message, "[%02d%02d %02d:%02d:%02d] %s ", lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, Filenameonly);

	if (state == 1) { //file is modified
		message[strlen(message)] = '\0';	
		strcat(message, "is modified");
	}
	else if (state == -1) { //file is deleted
		message[strlen(message)] = '\0';	
		strcat(message, "is deleted\n");
		size = strlen(message);
		write(fd, message, size);	
		kill(getpid(), SIGKILL);
		exit(1);
	}

	else;
	if((lt = localtime(&info.mtime)) == NULL) {
		fprintf(stderr, "localtime() call error\n");
		exit(1);
	}

	sprintf(message+strlen(message), " [size:%ld/mtime:%02d%02d %02d:%02d:%02d]\n", info.size, lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);	

	size = strlen(message);
	write(fd, message, size);
}
