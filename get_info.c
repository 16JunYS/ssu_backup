#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "ssu_backup.h"

int get_info(char *filename, backupInfo* info) {
	int mod = 0;
	struct stat statbuf;
	time_t t;
	struct tm *lt;
	char temp[10];

	if (stat(filename, &statbuf) < 0) { //file has been deleted or doesn't exist
		mod = -1;	
	}

	else if(statbuf.st_mtime != info->mtime) { //file has been modified
		info->mtime = statbuf.st_mtime;
		info->size = statbuf.st_size;
		mod = 1;
	}
	if ((t = time(NULL)) < 0) {
		fprintf(stderr, "time() call error\n");
		exit(1);
	}
	if ((lt = localtime(&t)) == NULL) {
		fprintf(stderr, "localtime() call error\n");
		exit(1);
	}

	info->backupt = t;
	sprintf(temp, "%02d%02d%02d%02d%02d", lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
	strcpy(info->btime, temp);

	return mod;
}
