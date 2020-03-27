#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char* Filenameonly;
char* backup_name(char *filename, char* btime) {
	char *curpath = malloc(200);
	char *pathname = malloc(200);
	char temp[50];
	char *new_fname;
	int size; //size of file's name
	int offset = 0, i;

	Filenameonly = malloc(200);
	if(filename[0] == '/' || filename[0] == '~') { //absolute path
		strcpy(pathname, filename);
		for(i = 0; i < strlen(filename); i++) {
			if(filename[i] == '/') offset = i;	
		}
		strcpy(Filenameonly, pathname+offset+1);
	}
	else if (filename[0] == '.') { //relative path
		for(i = 0; i < strlen(filename); i++) {
			if(filename[i] == '/') {
				offset = i;	
				strncpy(temp, filename, offset);
			}
		}
		strcpy(Filenameonly, filename+offset+1);
		getcwd(curpath, 200);
		chdir(temp);
		getcwd(pathname, 200);
		chdir(curpath);
		size = strlen(pathname);
		sprintf(pathname+size, "/%s", filename+offset+1);
	}
	else { //just the file name
		Filenameonly = filename;
		getcwd(curpath, 1024);
		sprintf(pathname, "%s/%s", curpath, filename);
	}
	
	if ((size = strlen(pathname)) > 122) {
		fprintf(stderr, "ssu_backup error:\nfilename is too long!\n");
		exit(1);
	}
	new_fname = (char *)malloc(255);
	for (i = 0; i < size; i++) {
		sprintf(new_fname+i*2, "%0x", pathname[i]);
	}
	i = i*2;
	sprintf(new_fname+i, "_%s", btime);

	return new_fname;
}
	
