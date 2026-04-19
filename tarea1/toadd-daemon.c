#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#define FIFOPATH "commands.txt"
#define LOGSPATH "toadd-daemon.log"


void write_log(char* info){
	FILE* file = fopen(LOGSPATH, "a");
	if(file == NULL){
		exit(EXIT_FAILURE);
	}
	time_t now;
	time(&now);
	struct tm *info_tiempo = localtime(&now);
	char timestamp[25];
	pid_t pid = getpid();
	strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S]", info_tiempo);
	fprintf(file, "%s [%d] %s\n", timestamp, pid, info);
	fclose(file);
}

//la logica de creacion de un daemon es: (1)crear proceso hijo ; (2)matar proceso padre; crear sesión (asi eliminamos tty del daemon) ; repetir (1) y (2)
static void init_daemon(){
	pid_t pid;
	pid = fork();
	if(pid<0){
		exit(EXIT_FAILURE);
	}
	if(pid>0){
		exit(EXIT_SUCCESS);
	}
	if(setsid() < 0){
		exit(EXIT_FAILURE);
	}
	pid = fork();
	if(pid<0){
		exit(EXIT_FAILURE);
	}
	if(pid>0){
		exit(EXIT_SUCCESS);
	}	
	umask(0);
	int x;
	for(x = sysconf(_SC_OPEN_MAX); x>=0; x--){
		close(x);
	}
	write_log("demonio iniciado correctamente");
}




void clean_fifo(){
	if(access(FIFOPATH, F_OK)==0){
		if(unlink(FIFOPATH) == -1){
			write_log("Error al borrar archivo. EXIT");
			exit(EXIT_FAILURE);
		}
		write_log("Eliminando commands.txt");
	}
}

void create_and_open_fifo(){
	clean_fifo();
	if(mkfifo(FIFOPATH, 0666) == -1){
		write_log("Error al crear FIFO");
		exit(EXIT_FAILURE);
	}
	write_log("Creado FIFO correctamente... Esperando Toadd-CLI");
	int fd = open(FIFOPATH, O_RDONLY);
	if(fd == -1){
		write_log("Error al abrir FIFO para lectura");
		clean_fifo();
		exit(EXIT_FAILURE);
	}
	write_log("FIFO abierto para lectura correctamente");
}

int main(){
	init_daemon();
	create_and_open_fifo();
	clean_fifo();

	return EXIT_SUCCESS;
}






