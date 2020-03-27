typedef int OPTION;
typedef int MOD;

typedef struct _backupInfo {
	size_t size;
	time_t mtime;
	time_t backupt;
	char btime[10];
}backupInfo;
