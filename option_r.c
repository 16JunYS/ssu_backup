#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include "ssu_backup.h"

extern char* backupName;
extern char* Filenameonly;

void option_r(char *filename) {
	struct dirent **namelist;
	struct stat statbuf;
	int fd, b_fd;
	int count, i, k=0, num, start = 0;
	char *_filename; //filename in hexadecimal
	char *backupf;
	char _btime[15][12]; //backup time
	char *line; //get line by line from backup_log.txt
	char c;
	int size;

	line = malloc(165);
	_filename = malloc(122);
	backupf = malloc(263);
	i = 0;
	memset(_filename, '\0', 122);
	while(backupName[i] != '_') { //filename_16
		_filename[i] = backupName[i];
		i++;
	}
	size = strlen(_filename);
	if ((count = scandir("./backup", &namelist, NULL, alphasort)) == -1) {
		fprintf(stderr, "/backup directory scan error\n");
		exit(1);
	}
	for (i = 0; i < count; i++) {
		if (strncmp(_filename, namelist[i]->d_name, size) == 0) { //backup file exists
			if (k == 0) {
				printf("0. exit\n");
				k++;
			}
			strcpy(_btime[k], (namelist[i]->d_name)+strlen(_filename));
			_btime[k][11] = '\0';
			sprintf(backupf, "./backup/%s%s", _filename, _btime[k]);
			stat(backupf, &statbuf);
			sprintf(line, "%d. %s_%s [size:%ld]", k++, Filenameonly, _btime[k], statbuf.st_size);
			printf("%s\n", line);
		}
		else continue;
	}
	printf("input : ");
	scanf("%d", &num);
	sprintf(backupf, "./backup/%s%s", _filename, _btime[num]);
	if (num == 0) exit(0);
	else {
		printf("[%s]\n", Filenameonly);
		fd = open(filename, O_RDWR | O_TRUNC, 0644);
		b_fd = open(backupf, O_RDONLY);
		while(read(b_fd, &c, 1) > 0) {
			write(fd, &c, 1);
			printf("%c", c);
		}
	}

	for (i = 0; i < count; i++) 
		free(namelist[i]);
	free(namelist);
	free(line);
	free(_filename);
	free(backupf);
	exit(0);
}
