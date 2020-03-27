#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h> 
#include <pthread.h>
#include <string.h>
#include "ssu_backup.h"

extern char* logfile;
extern char* Filenameonly;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *backup_thread(void *arg);
void write_log(backupInfo info, int state);
void ssu_copy(char *source, char *backupfile, backupInfo info);
int get_info(char *filename, backupInfo* info);

typedef struct _thread_data {
	char dirname[122];
	char dir_hex[244];
	char *btime;
	time_t backupt;
} thread_data;

	thread_data data;
void option_d(char *_dirname, char *_dir_hex, char *btime, time_t backupt) {
	pthread_t tid[500];	
	int cnt_dir, cnt, size, size_hex;
	struct dirent **namelist;
	struct stat statbuf;
	char dirname[122];
	char dir_hex[244];
	char temp[122];
	int status;	
	int i;

	strcpy(dirname, _dirname);
	strcpy(dir_hex, _dir_hex);
	if ((cnt_dir = scandir(dirname, &namelist, NULL, alphasort)) == -1) { //scan directory
		fprintf(stderr, "%s directory scan error\n", dirname);
		exit(1);
	}
	size = strlen(dirname);
	size_hex = strlen(dir_hex);
	dirname[size] = '\0';
	dir_hex[size_hex] = '\0';
	for (cnt = 0; cnt < cnt_dir; cnt++) { //scan files in directory user entered
		memset(dirname+size, '\0', 122-size);
		memset(dir_hex+size_hex, '\0', 244-size);
		if (strcmp(namelist[cnt]->d_name, ".") == 0 || strcmp(namelist[cnt]->d_name, "..") == 0) 
			continue;
		strcat(dirname, namelist[cnt]->d_name); //"[directory] + filename"
		

		//make backupfile name in 16 form
		memset(temp, '\0', 122);
		for (i = 0; i < strlen(namelist[cnt]->d_name); i++) {
			sprintf(temp+i*2, "%0x", namelist[cnt]->d_name[i]);
		}
		temp[strlen(temp)] = '\0';
		strcat(dir_hex, temp); //dir_hex : directory/file in hexadecimal form
		stat(dirname, &statbuf);
		if (S_ISDIR(statbuf.st_mode)) { //if there is sub directory
			dirname[strlen(dirname)] = '/';
			dir_hex[strlen(dir_hex)] = '\0';
			strcat(dir_hex, "2f");
			dir_hex[strlen(dir_hex)] = '\0';	
			option_d(dirname, dir_hex, btime, backupt); 
			
			continue;	
		}
		
		dir_hex[strlen(dir_hex)] = '_';
		strcat(dir_hex, btime);
		if(strlen(dir_hex) > 255) {
			fprintf(stderr, "ssu_backup error\n");
			fprintf(stderr, "backup file [%s] name cannot be over 255 bytes\n", namelist[cnt]->d_name);
			continue;
		}
			//copy data to use them in new thread
		memset(data.dirname, '\0', 122);
		strcpy(data.dirname, dirname);
		memset(data.dir_hex, '\0', 244);
		strcpy(data.dir_hex, dir_hex);
		data.btime = btime;
		Filenameonly = namelist[cnt]->d_name;
		data.backupt = backupt;

		if(pthread_create(&tid[cnt], NULL, backup_thread, (void *)&data) != 0) {
			fprintf(stderr, "pthread_create error\n");
			continue;
		}
		pthread_join(tid[cnt], NULL);
	}
//	status = pthread_mutex_destroy(&mutex);
	return;
}

void *backup_thread(void *arg) {
	thread_data data = *(thread_data *)arg;
	backupInfo info;
	int mod;
	pthread_mutex_lock(&mutex);
	mod = get_info(data.dirname, &info);

	if (mod == -1) mod = 1; //file has been deleted -> write on log file "modified"
	else if (mod == 1) mod = 0;
	strcpy(info.btime, data.btime);
	info.backupt = data.backupt;
	write_log(info, mod);
	if (mod == 1) { // deleted file
	}
	else {
		ssu_copy(data.dirname, data.dir_hex, info);
	}
	pthread_mutex_unlock(&mutex);
//	pthread_exit(NULL);
}
