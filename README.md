# ssu_backup
File backup program using daemon process

디몬 프로세스는 사용자가 직접 제어하지 않고, 백그라운드 상태에서 반복적으로 작업 수행 하는 프로그램으로, ssu_backup 은 Linux 환경에서 디몬 프로세스를 생성하여 주기적으로 파일을 백업하는 기능을 하는 프로그램이다. 

## user guide

### syntax
```
ssu_backup [filename] [period] [option] // filename을 period 주기에 따라 백업
```
[filename]은 특별한 옵션이 없는 경우에는 일반파일이고, [period]는 범위가 3에서 10까지 이다. ssu_backup 을 실행 했을 때 이미 실행 중인 ssu_backup 디몬 프로세스가 있는 경우에는 사용자 정의 시그널을 발생시켜서 종료시키고, 백업을 진행하면서 ‘backup_log.txt’의 로그 파일에 과정을 기록하도록 한다.

백업파일 이름은 백업대상의 절대경로를 16진수 아스키값으로 변환하고 “_백업시간”을 붙인다. 종료된 프로세스에 관해서 남긴 로그에는 종료된 pid 를 기록한다. 

### 다음과 같은 경우에 에러가 발생한다.

1. `filename` 이 존재 하지 않는 경우
2. regular file 이 아닌 경우
3. 백업 주기 범위 (3 ~ 10) 를 벗어난 경우
4. `'-r'` 옵션이 다른 옵션과 함께 입력된 경우
5. `'-d'` 옵션으로 입력된 백업 대상이 디렉터리가 아닌 경우

①	백업대상 파일이 수정된 경우
②	백업대상 파일이 삭제된 경우

## 구조 설계
1)	main 함수 :
①	main 함수의 인자를 받아 백업 대상 파일, 백업 주기, 옵션을 받아 변수에 저장한다. (option( ) 사용)
②	에러가 없는 경우 디몬 프로세스를 생성한다(start_daemon( )). 생성하기 전 이미 실행 중인 ssu_backup 디몬 프로세스가 있는 경우 종료한다(fnd_proc( )).
③	백업 대상파일의 파일 정보를 저장한 후(get_info( )), 백업 될 파일을 복사하여 새 백업 파일을 만들어낸다(backup_name( ), ssu_copy( )). 백업을 수행한 후 결과를 로그파일에 남긴다(write_log( )). 이 과정을 [period]에 맞춰 반복해서 백업한다.
옵션 ‘-d’가 적용 된 경우 디렉터리 안의 하위 파일들을 차례대로 백업하여 처리한다(option_d( )).
④	 옵션 ‘-r’ : 백업 대상 파일을 16진수화 한 이름과 백업 된 폴더 파일들과 비교해서 복구할 백업파일을 출력한다 (backup_name( ), option_r( ))


## 구현 (각 함수별 기능)
가)	ssu_backup.c
1)	int main(int argc, char *argv[])
①	매개 변수 : argc, argv[]
i.	argc : 명령 줄에서 입력된 명령행 옵션들의 개수를 저장한다.
ii.	argv[] : 명령 줄에서 입력된 옵션들과 백업될 파일, 백업 주기를 받아온다.
②	기능
백업될 파일을 디몬 프로세스를 통해 반복적으로 백업한다.
사용자가 프로그램을 처음 시작하면, option()을 호출하여 옵션과 백업파일, 백업 주기 등의 정보를 받는다. 지정한 변수에 값들을 저장한 후, 기존에 실행 중인 ./ssu_backup 디몬 프로세스가 있는지 fnd_process()를 호출하여 확인한다. 그 후, 옵션에 맞게 진행한다.
‘-r’이 적용 된 경우, [period]는 받지 않는 것으로 간주하고 백업 파일을 ‘filename’변수에 저장한다. 그리고 해당 파일의 파일 정보(수정 시간, 크기, 백업한 시간)를 받아와 “ssu_backup.h”에 저장되어 있는 구조체를 이용하여 값들을 저장한다(get_info()). 변수 ‘backupName’에 백업 대상 파일의 절대경로를 16진수화 하여 백업 파일의 이름을 새로 만들고(backup_name()), option_r()을 호출해 해당 옵션 기능을 작동시킨다.
그 외의 경우 [period], [filename]을 변수 ‘period’, [filename]에 각각 저장한다. ‘-d’ 옵션이 있는 경우 우선 해당 파일이 디렉터리인지 확인하여 에러가 나지 않게끔 한다. ‘-d’ 옵션이 적용 되지 않은 경우에는 반대로 해당 파일이 일반파일이 맞는지 확인하여 에러가 나지 않게끔 한다. 에러가 나지 않는 지 확인한 후, 디몬 프로세스를 생성한다(‘-r’, ‘-c’ 옵션은 제외).
나)	kill_process.c
1)	fnd_proc();
①	기능 : 이미 실행 중인 ssu_backup 디몬 프로세스가 있는 지 확인한다.
SIGUSR1 시그널이 발생한 경우 ‘kill_handler()’ 사용자 핸들러 함수를 호출하여 처리하도록 설정한다. 디렉터리 ‘/proc’을 살펴보면 진행 중인 프로세스의 pid의 이름으로 된 하위 디렉터리들이 만들어져 있는 것을 알 수 있다. scandir()함수를 통해 ‘/proc’ 디렉터리 값들을 변수 ‘namelist’에 정리하여 저장하고 차례대로 디렉터리 내부 stat을 살펴봐서 ssu_backup이 써져 있는 경우 이미 실행 중인 프로세스가 있다는 것을 의미하므로 raise()를 통해 자신에게 시그널 SIGUSR1을 발생시킨다. 시그널 SIGUSR1이 발생했으므로 핸들러 함수를 호출하여 종료하도록 한다.
2)	void kill_handler(int signo);
①	기능 : 시그널 SIGUSR1이 발생했으므로 기존 디몬 프로세스를 종료한다.
backup_log.txt 로그 파일에 현재 시간을 받아와 종료시점을 남기고 getpid()로 죽은 프로세스의 pid를 구해 기록한다. 자신에게 SIGKILL 시그널을 보내 실행중인 ssu_backup 디몬 프로세스를 죽인다.
다)	get_info.c
1)	int get_info(char *filename, backupInfo *info);
①	매개 변수 : filename, info
i.	filename : 백업 대상 파일의 이름
ii.	info : 백업 대상 파일의 파일 정보를 저장할 구조체
②	리턴 값 : 해당 파일이 수정된 경우 1, 삭제 된 경우 -1, 어떠한 변화도 없는 경우 0을 리턴 한다.
③	기능 : 백업 대상 파일의 파일 정보를 받아온다.
파일의 수정시간과 크기를 stat()함수를 통해 받아와 info->mtime과 info->size에 저장한다. 그리고 time()함수를 통해 현재 백업 중인 시간을 구해 info.->backupt에 저장하고, “MMDDHHMMSS” 형식에 맞게 문자열에 저장한 후 info->btime에 저장한다.
라)	backup_name.c
1)	char* backup_name(char *filename, char *btime);
①	매개 변수 : filename, btime
i.	filename : 백업 대상의 파일 이름
ii.	btime : 백업되는 시간을 저장한 문자열(MMDDHHMMSS)
②	리턴 값 : 백업될 파일의 이름 (“백업 대상의 절대경로를 16진수화 한 값_백업시간”)
③	기능 : 새로 백업 될 파일의 이름을 만든다.
사용자가 입력한 파일이 절대 경로로 되어있는지, 상대 경로로 되어 있는 지 파악한 후 모두 절대 경로로 바꿔서 변수 ‘pathname’에 저장한다. 그리고 ‘pathname’의 문자를 한글자씩 읽어서 각각 16진수 아스키 값으로 나타내어 변수 ‘new_fname’에 저장한다. 저장한 ‘new_fname’뒤에는 “_”와 함께 인자로 받아온 btime을 붙여서 백업될 파일의 이름을 만든다.
마)	ssu_cp.c
1)	void ssu_cp(char *source, char *backupfile, backupInfo info);
①	매개 변수 : source, backupfile, info
i.	source : 백업 될 파일
ii.	backupfile : 새로 백업되어 만들어질 파일의 이름
iii.	info : 백업 대상 파일의 정보가 담긴 구조체
②	기능 : open()을 통해 백업 될 파일을 열어 파일을 읽어오고, 읽어 온 내용은 write()를 통해 ‘backupfile’이름의 새로운 백업 파일에 복사를 한다.
바)	write_log.c
1)	void write_log(backupInfo info, int state);
①	매개 변수 : info, state¬
i.	info : 백업 할 파일의 정보를 담고 있는 구조체
ii.	state : get_info에서 리턴 한 값. (0:수정/삭제가 일어나지 않은 경우, 1:수정, -1:삭제)
②	기능 : 백업 파일이 백업됨을 알리는 로그 메시지를 ‘backup_log.txt’에 기록한다.
‘info.backupt’를 이용하여 백업 시간과 파일 이름을 출력하고, state이 1인 경우 “is modified”, -1인 경우 “is deleted”, 0인 경우 파일의 사이즈와 수정 시간을 형식에 맞게 출력한다.
사)	option_d.c
1)	void option_d(char *_dirname, char *_dir_hex, char *btime, time_t backup) {
①	매개 변수 : _dirname, _dir_hex, btime, backup
i.	_dirname : 입력받은 디렉터리의 절대경로
ii.	_dir_hex : 디렉터리의 절대경로를 ASCII 값으로 16진수화 한 값
iii.	btime : 백업 시간(MMDDHHMMSS 형식)
iv.	backup : 백업 시간
②	기능 : ‘-d’ 옵션에 맞게 백업 될 디렉터리의 내부 파일들의 스레드를 각각 생성하여 백업한다.
구조체 thread_data tid[]에 _dirname, _dir_hex, btime, backup 값을 저장해서 쓰레드가 생성되면 자원을 공유하면서 잘못된 정보로 백업처리되지 않게끔 한다. 입력받은 디렉터리내부 파일들을 scandir()을 통해 namelist에 저장하고, 각각 파일들이 디렉터리인지 일반파일인지 확인한다. 디렉터리인 경우, 해당 디렉터리의 이름으로 다시 option_d()를 호출해서 서브디렉토리에 있는 파일까지 백업처리를 할 수 있도록 한다. 일반 파일임이 확인 되면 pthread_create()를 통해 스레드를 생성하고 backup_thread()함수를 실행한다. 해당 함수로 넘겨지는 인자는 thread_data tid 이다.
2)	void *backup_thread(void *arg)
①	기능 : 생성된 쓰레드에 해당하는 파일을 백업한다. 주어진 옵션이 없을 때 백업하는 순서대로 get_info()를 통해 파일 정보를 받아오고, backup_name()을 사용하여 만들어질 백업파일의 이름을 만들어낸다. 그리고 write_log()와 ssu_cp()를 호출하여 백업 및 백업 관련 내용을 로그 파일에 기록한다.
아)	option_r.c
1)	void option_r(char *filename)
①	매개 변수 filename : 복구할 파일의 이름
②	기능 : ‘-r’ 기능을 수행한다. 입력한 파일을 백업한 백업 파일들을 차례대로 출력해 사용자가 선택한 백업 파일로 복구한다.
입력 받은 filename의 절대경로를 구해 ASCII 값의 16진수화하여 ‘_filename’에 저장한다. 그리고 ‘./backup’ 디렉터리의 파일들과 차례대로 비교하여 같은 이름이 있는 지 확인한다. 백업 파일들은 16진수화한 값과 함께 백업 시간도 저장되어 있으므로 백업 시간이 써져 있는 부분 전까지만을 비교한다. 같은 이름의 파일들은 ‘1’번부터 “이름_백업시간”, 백업 파일 크기를 차례대로 출력한다. 사용자로부터 복구하고 싶은 파일의 번호를 받아온다. 복구할 파일을 읽어와 문자열에 저장한 후, 원본 파일은 덮어쓰는 방식으로 write()를 사용하여 복구하고, 복구된 파일의 내용을 출력한다.
