#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <syslog.h>
#include <string.h>
#include "ssu_backup.h"

#define FNAME 255

OPTION on  = 0;
OPTION c = 0;
OPTION r = 0;
OPTION m = 0;
OPTION n = 0;
OPTION d = 0;
int period;
char *backupName;
extern char *Filenameonly; //used to change its name(10->16 form)

void fnd_proc(); //find out if there is running ssu_backup daemon process
void kill_handler(int signo);
int start_daemon(); //start daemon
void option(int argc, char *argv[], char *filename); //handle option
int get_info(char *filename, backupInfo* info); //get backupfile information
void write_log(backupInfo info, int); //write backup file info. on log file
char* backup_name(char *filename, char *btime); //make backupfile's name
void ssu_copy(char *source, char *target, backupInfo info);
void option_r(char *filename);
void option_d(char *_dirname, char *_dir_hex, char *btime, time_t backupt);
int main(int argc, char *argv[]) 
{
	char *filename = malloc(123);
	int fd;
	backupInfo info;
	int mod;
	int i;
	char *dir_abs, *dir_hex, *curpath;
	int size;
	if (argc < 3) {
		fprintf(stderr, "argument is not enough\n");
		exit(1);
	}
	backupName = malloc(FNAME);
	
	mkdir("backup", 0755);
	option(argc, argv, filename);
	if (!on) {
		mod = get_info(filename, &info);
		while(1) {
			openlog("lpd", LOG_PID, LOG_LPR);
			syslog(LOG_ERR, "open failed lpd %m");
			closelog();
			backupName = backup_name(filename, info.btime); //rename backup file	
			mod = get_info(filename, &info); //save backup file information
			write_log(info, mod); //write on the log file	
			ssu_copy(filename, backupName, info); //backup file as 'backupName'	
			sleep(period);
		}
	}
	
	if(d) { //option d
		dir_abs = malloc(123);
		if (filename[0] == '/' || filename[0] == '~') { //filename is in absolute form
			strcpy(dir_abs, filename);
		}
		else if (filename[0] == '.') { //filename is in relative form
			curpath = malloc(100);
			getcwd(curpath, 100);
			chdir(filename);
			getcwd(dir_abs, 123);
			chdir(curpath);
		}
		else { //just the directory name
			getcwd(curpath, 100);
			sprintf(dir_abs, "%s/%s", curpath, filename);
		}
		size = strlen(dir_abs);
		dir_abs[size] = '\0';
		strcat(dir_abs, "/");
		printf("dir : %s\n", dir_abs);

		dir_hex = malloc(255);
		size = strlen(dir_abs);		
		for (i = 0; i < size; i++) 
			sprintf(dir_hex+i*2, "%0x", dir_abs[i]);
		dir_hex[i*2] = '\0';

		while(1) {
			openlog("lpd", LOG_PID, LOG_LPR);
			syslog(LOG_ERR, "open failed for %s lpd %m", dir_abs);
			closelog();
			get_info(dir_abs, &info);
			option_d(dir_abs, dir_hex, info.btime, info.backupt);
			sleep(period);
		}
	}
	else {
	}
	exit(0);
}

void option(int argc, char *argv[], char *filename) {
	int opt;
	backupInfo info;
	int pid;	
	int mod;
	int fd;
	struct stat statbuf;

	while((opt = getopt(argc, argv, "mn:dcr")) != -1) {
		switch(opt) {
			case 'm' : //back up only when backup file is modified
				if (r) printf("ssu_backup error:\noption r cannot be used with other options\n");
				if (c) printf("ssu_backup error:\noption c cannot be used with other options\n");
				if (r||c) exit(1);
				on++;	
				break;
			case 'n' : //back up leaving latest N backupfiles
				if (r) printf("ssu_backup error:\noption r cannot be used with other options\n");
				if (c) printf("ssu_backup error:\noption c cannot be used with other options\n");
				if (r||c) exit(1);
				on++;
				break;
			case 'd' : //back up all files in the directory
				if (r) printf("ssu_backup error:\noption r cannot be used with other options\n");
				if (c) printf("ssu_backup error:\noption c cannot be used with other options\n");
				if (r||c) exit(1);
				d++;
				on++;
				break;
			case 'c' : //print backup file and compare with original backup file 
				if(on) {
					fprintf(stderr, "ssu_backup error :\n");
					fprintf(stderr, "option c cannot be used with another option\n");
					exit(1);
				}
				on++;
				c++;
				break;
			case 'r' : //print backup time, file size and let user choose backup files
				if(on) {
					fprintf(stderr, "ssu_backup error :\n");
					fprintf(stderr, "option c cannot be used with another option\n");
					exit(1);
				}
				on++;
				r++;
				
				break;
		}
	}
	pid = getpid();
	fnd_proc();
	if (c) {
		strcpy(filename, argv[argc-1]);
		mod = get_info(filename, &info); //get file's information
		backupName = backup_name(filename, info.btime); //get backup file Name
		exit(0);		
	}
	else if (r) {
		// print backuped-files and select the file user want to backup
		strcpy(filename, argv[argc-1]);
		mod = get_info(filename, &info);
		if (mod == -1) {
			printf("file \"%s\" doesn't exist\n", filename);
			exit(0);	
		}
		backupName = malloc(FNAME);
		backupName = backup_name(filename, info.btime);
		option_r(filename);
		exit(0);
	}
	else {
		period = atoi(argv[argc-1]);
		if (period < 3 || period > 10) {
			fprintf(stderr, "ssu_backup error:\n");
			fprintf(stderr, "[PERIOD] : 3~10\n");
			exit(1);
		}
		strcpy(filename, argv[argc-2]);
		if (d) {
			stat(filename, &statbuf);
			if (S_ISDIR(statbuf.st_mode) == 0) {
				fprintf(stderr, "ssu_backup error\n");
				fprintf(stderr, "file \"%s\" must be directory with option d\n", filename);
				exit(1);
			}
		}
		else if ((fd = open(filename, O_RDONLY)) < 0) {
			//error if file does not exist
			fprintf(stderr, "\"%s\" backup file does not exist\n", filename);
			exit(1);
		}
		else {
			stat(filename, &statbuf);
			if (S_ISREG(statbuf.st_mode) == 0) {
				fprintf(stderr, "ssu_backup error\n");
				fprintf(stderr, "file \"%s\" must be regular file\n", filename);
				exit(1);
			}
		}
		if (start_daemon() < 0) {
			fprintf(stderr, "ssu_daemon error\n");
			exit(1);
		}
	}
}
int start_daemon() {
	pid_t pid;
	int fd, maxfd;

	if ((pid = fork()) < 0) {
		fprintf(stderr, "fork error\n");
		exit(1);
	}

	else if (pid != 0) {
		exit(0); //terminate parent process
	}

	pid = getpid();
	printf("process %d running as ssu_backup daemon.\n", pid);
	setsid();
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	maxfd = getdtablesize();

	for (fd = 0; fd < maxfd; fd++)
		close(fd);

	umask(0);
	//chdir("/");
	fd = open("/dev/null", O_RDWR);
	dup(0);
	dup(0);
	return 0;
}
