#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <string.h>
#include "ssu_backup.h"

void ssu_copy(char *source, char *backupfile, backupInfo info) {
	int fd_src, fd_tar;
	struct stat stat_src, stat_tar;
	struct utimbuf time_buf;
	char *target;
	char c;

	target = malloc(260);
	strcpy(target, "./backup/\0");
	strcat(target, backupfile);

	if ((fd_src = open(source, O_RDONLY)) < 0) {
		fprintf(stderr, "open error for %s\n", source);
		exit(1);
	}

	if ((fd_tar = open(target, O_WRONLY | O_CREAT, 0644)) < 0) { //create backup file
		fprintf(stderr, "open error for %s\n", target);
		exit(1);
	}

	if (stat(target, &stat_tar) < 0) {
		fprintf(stderr, "stat error for %s\n", target);
		exit(1);
	}
	while(read(fd_src, &c, 1) > 0) { //copy backup file
		write(fd_tar, &c, 1);
	}

	time_buf.actime = stat_tar.st_atime;
	time_buf.modtime = info.mtime;
	utime(target, &time_buf);
}
