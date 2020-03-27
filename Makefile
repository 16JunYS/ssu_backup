ssu_backup : ssu_backup.o backup_name.o get_info.o write_log.o ssu_cp.o option_r.o kill_process.o option_d.o
	gcc -o ssu_backup ssu_backup.o backup_name.o get_info.o write_log.o ssu_cp.o option_r.o kill_process.o option_d.o -lpthread

ssu_backup.o : ssu_backup.c
	gcc -c ssu_backup.c

backup_name.o : backup_name.c
	gcc -c backup_name.c
get_info.o : get_info.c
	gcc -c get_info.c

write_log.o : write_log.c
	gcc -c write_log.c
ssu_cp.o : ssu_cp.c
	gcc -c ssu_cp.c
option_r.o : option_r.c
	gcc -c option_r.c
kill_process.o : kill_process.c
	gcc -c kill_process.c
option_d.o : option_d.c
	gcc -c option_d.c
