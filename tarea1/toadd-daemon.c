#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>

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
	openlog("toadd-daemon", LOG_PID, LOG_DAEMON);
}

int main(){
	init_daemon();
	while(1){
		syslog(LOG_NOTICE, "hola mundo desde el demonio");
		sleep(20);
		break;
	}

	syslog(LOG_NOTICE, "se terminó this shit");
	closelog();

	return EXIT_SUCCESS;
}






