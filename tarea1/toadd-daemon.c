#define _POSIX_C_SOURCE 200809L
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
#include <string.h>
#include <sys/wait.h>

#define FIFOPATH "commands.txt"
#define RESPATH "res.txt"
#define LOGSPATH "toadd-daemon.log"
#define MAX_PROCESSES 100

typedef enum{
	RUNNING,
	STOPPED,
	ZOMBIE
} process_state_t;

typedef struct{
	int iid;
	pid_t pid;
	process_state_t state;
	time_t start_time;
	char binary_path[256];
	int is_active;
} child_process_t;

child_process_t process_table[MAX_PROCESSES];
int next_iid = 2;

void write_log(const char* info){
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

void add_process_to_table(pid_t pid, const char* path) {
    int encontrado = 0;
    char log_buffer[512];

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].is_active == 0) {
            process_table[i].iid = next_iid++;
            process_table[i].pid = pid;
            process_table[i].state = RUNNING;
            process_table[i].start_time = time(NULL);

            strncpy(process_table[i].binary_path, path, sizeof(process_table[i].binary_path) - 1);
            process_table[i].binary_path[sizeof(process_table[i].binary_path) - 1] = '\0';

            process_table[i].is_active = 1;

            snprintf(log_buffer, sizeof(log_buffer),
                     "Proceso agregado: IID %d, PID %d, Path %s",
                     process_table[i].iid, (int)pid, path);
            write_log(log_buffer);

            encontrado = 1;
            break;
        }
    }

    if (!encontrado) {
        write_log("[ERROR] No se pudo agregar proceso: tabla llena.");
    }
}

void handle_sigchld(int sig) {
    (void)sig;
}

void check_zombies() {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].is_active && process_table[i].pid == pid) {
                if (process_table[i].state == RUNNING) {
                    process_table[i].state = ZOMBIE;
                    char log_buffer[256];
                    snprintf(log_buffer, sizeof(log_buffer),
                             "Proceso IID %d (PID %d) termino. Estado actualizado a ZOMBIE.",
                             process_table[i].iid, (int)pid);
                    write_log(log_buffer);
                }
                break;
            }
        }
    }
}

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
	int null_fd = open("/dev/null", O_RDWR);
	if (null_fd != -1) {
		dup2(null_fd, 0);
		dup2(null_fd, 1);
		dup2(null_fd, 2);
		if (null_fd > 2) close(null_fd);
	}
	write_log("Demonio iniciado correctamente");
}

void init_process_table(){
	for(int i=0; i<MAX_PROCESSES; i++){
		process_table[i].is_active = 0;
	}
}

void clean_fifo(){
	if(access(FIFOPATH, F_OK)==0){
		if(unlink(FIFOPATH) == -1){
			write_log("[ERROR] Error al borrar FIFO");
			exit(EXIT_FAILURE);
		}
		write_log("FIFO commands.txt eliminado");
	}
}

void create_and_open_fifo(int *fd_ptr){
	clean_fifo();
	if(mkfifo(FIFOPATH, 0666) == -1){
		write_log("[ERROR] Error al crear FIFO");
		exit(EXIT_FAILURE);
	}
	write_log("FIFO creado. Esperando conexion de toadd-cli...");
	do {
	    *fd_ptr = open(FIFOPATH, O_RDONLY);
	} while (*fd_ptr == -1 && errno == EINTR);

	if(*fd_ptr == -1){
		write_log("[ERROR] Error al abrir FIFO para lectura");
		clean_fifo();
		exit(EXIT_FAILURE);
	}
	write_log("FIFO abierto para lectura");
}

int read_buffer(int fd, char *dest, int max_len){
	memset(dest, 0, max_len);
	ssize_t n = read(fd, dest, max_len -1);
	if(n>0){
		dest[n] = '\0';
		return (int)n;
	}
	return (int)n;
}

void start(const char* binary_path) {
    char log_buffer[512];

    if (access(binary_path, X_OK) != 0) {
        snprintf(log_buffer, sizeof(log_buffer),
                 "[ERROR] El binario '%s' no existe o no es ejecutable: %s",
                 binary_path, strerror(errno));
        write_log(log_buffer);
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        snprintf(log_buffer, sizeof(log_buffer),
                 "[ERROR] fork() fallo para: %s", binary_path);
        write_log(log_buffer);
        return;
    }
    else if (pid == 0) {
        setpgid(0, 0);
        execlp(binary_path, binary_path, NULL);
        char err_buf[512];
        snprintf(err_buf, sizeof(err_buf),
                 "[ERROR] execlp fallo para '%s': %s", binary_path, strerror(errno));
        write_log(err_buf);
        _exit(EXIT_FAILURE);
    }
    else {
        snprintf(log_buffer, sizeof(log_buffer),
                 "Binario '%s' iniciado con PID: %d", binary_path, pid);
        write_log(log_buffer);
        add_process_to_table(pid, binary_path);
    }
}

void stop(int iid) {
    char log_buffer[512];
    int encontrado = 0;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].is_active && process_table[i].iid == iid) {
            encontrado = 1;
            if (kill(process_table[i].pid, SIGTERM) == 0) {
                process_table[i].state = STOPPED;
                snprintf(log_buffer, sizeof(log_buffer),
                         "SIGTERM enviado. Proceso detenido: IID %d, PID %d",
                         iid, (int)process_table[i].pid);
                write_log(log_buffer);
            } else {
                snprintf(log_buffer, sizeof(log_buffer),
                         "[ERROR] No se pudo enviar SIGTERM al IID %d (PID %d): %s",
                         iid, (int)process_table[i].pid, strerror(errno));
                write_log(log_buffer);
            }
            break;
        }
    }
    if (!encontrado) {
        snprintf(log_buffer, sizeof(log_buffer),
                 "[ERROR] No se encontro proceso activo con IID %d.", iid);
        write_log(log_buffer);
    }
}

void kill_process(int iid) {
    char log_buffer[512];
    int encontrado = 0;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].is_active && process_table[i].iid == iid) {
            encontrado = 1;
            pid_t pid = process_table[i].pid;

            if (kill(-pid, SIGKILL) == 0) {
                snprintf(log_buffer, sizeof(log_buffer),
                         "SIGKILL enviado al grupo de procesos IID %d (PGID %d)",
                         iid, (int)pid);
                write_log(log_buffer);
            } else {
                if (kill(pid, SIGKILL) == 0) {
                    snprintf(log_buffer, sizeof(log_buffer),
                             "SIGKILL enviado al proceso IID %d (PID %d)",
                             iid, (int)pid);
                    write_log(log_buffer);
                } else {
                    snprintf(log_buffer, sizeof(log_buffer),
                             "[ERROR] No se pudo enviar SIGKILL al IID %d (PID %d): %s",
                             iid, (int)pid, strerror(errno));
                    write_log(log_buffer);
                }
            }

            waitpid(pid, NULL, 0);

            process_table[i].is_active = 0;
            snprintf(log_buffer, sizeof(log_buffer),
                     "Proceso IID %d (PID %d) eliminado de la tabla.",
                     iid, (int)pid);
            write_log(log_buffer);
            break;
        }
    }
    if (!encontrado) {
        snprintf(log_buffer, sizeof(log_buffer),
                 "[ERROR] No se encontro proceso activo con IID %d para kill.", iid);
        write_log(log_buffer);
    }
}

void show_processes() {
    int fd;
    do {
        fd = open(RESPATH, O_WRONLY);
    } while (fd == -1 && errno == EINTR);

    if (fd == -1) {
        write_log("[ERROR] No se pudo abrir res.txt para escritura.");
        return;
    }

    char buffer[2048];
    snprintf(buffer, sizeof(buffer), "%-5s %-8s %-10s %-8s %s\n", "IID", "PID", "STATE", "UPTIME", "BINARY");
    if (write(fd, buffer, strlen(buffer)) == -1) {
        write_log("[ERROR] Fallo al escribir encabezado en ps.");
        close(fd);
        return;
    }

    time_t now = time(NULL);
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].is_active) {
            char state_str[16];
            switch(process_table[i].state) {
                case RUNNING: strcpy(state_str, "RUNNING"); break;
                case STOPPED: strcpy(state_str, "STOPPED"); break;
                case ZOMBIE:  strcpy(state_str, "ZOMBIE"); break;
                default:      strcpy(state_str, "UNKNOWN"); break;
            }
            int diff_seconds = (int)difftime(now, process_table[i].start_time);
            int h = diff_seconds / 3600;
            int m = (diff_seconds % 3600) / 60;
            int s = diff_seconds % 60;

            snprintf(buffer, sizeof(buffer), "%-5d %-8d %-10s %02d:%02d:%02d %s\n",
                     process_table[i].iid,
                     (int)process_table[i].pid,
                     state_str,
                     h, m, s,
                     process_table[i].binary_path);
            if (write(fd, buffer, strlen(buffer)) == -1) {
                write_log("[ERROR] Fallo al escribir fila en ps.");
                break;
            }
        }
    }
    close(fd);
    write_log("Tabla de procesos enviada a res.txt");
}

void show_status(int iid) {
    int fd;
    do {
        fd = open(RESPATH, O_WRONLY);
    } while (fd == -1 && errno == EINTR);

    if (fd == -1) {
        write_log("[ERROR] No se pudo abrir res.txt para status.");
        return;
    }

    char buffer[2048];
    snprintf(buffer, sizeof(buffer), "%-5s %-8s %-10s %-8s %s\n", "IID", "PID", "STATE", "UPTIME", "BINARY");
    if (write(fd, buffer, strlen(buffer)) == -1) {
        write_log("[ERROR] Fallo al escribir encabezado en status.");
        close(fd);
        return;
    }

    time_t now = time(NULL);
    int encontrado = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].is_active && process_table[i].iid == iid) {
            encontrado = 1;
            char state_str[16];
            switch(process_table[i].state) {
                case RUNNING: strcpy(state_str, "RUNNING"); break;
                case STOPPED: strcpy(state_str, "STOPPED"); break;
                case ZOMBIE:  strcpy(state_str, "ZOMBIE"); break;
                default:      strcpy(state_str, "UNKNOWN"); break;
            }
            int diff_seconds = (int)difftime(now, process_table[i].start_time);
            int h = diff_seconds / 3600;
            int m = (diff_seconds % 3600) / 60;
            int s = diff_seconds % 60;

            snprintf(buffer, sizeof(buffer), "%-5d %-8d %-10s %02d:%02d:%02d %s\n",
                     process_table[i].iid,
                     (int)process_table[i].pid,
                     state_str,
                     h, m, s,
                     process_table[i].binary_path);
            if (write(fd, buffer, strlen(buffer)) == -1) {
                write_log("[ERROR] Fallo al escribir fila en status.");
            }
            break;
        }
    }
    if (!encontrado) {
        snprintf(buffer, sizeof(buffer), "Proceso con IID %d no encontrado.\n", iid);
        if (write(fd, buffer, strlen(buffer)) == -1) {
            write_log("[ERROR] Fallo al escribir mensaje de no encontrado.");
        }
    }
    close(fd);
    write_log("Informacion de status enviada a res.txt");
}

void show_zombies() {
    int fd;
    do {
        fd = open(RESPATH, O_WRONLY);
    } while (fd == -1 && errno == EINTR);

    if (fd == -1) {
        write_log("[ERROR] No se pudo abrir res.txt para zombie.");
        return;
    }

    char buffer[2048];
    snprintf(buffer, sizeof(buffer), "%-5s %-8s %-10s %-8s %s\n", "IID", "PID", "STATE", "UPTIME", "BINARY");
    if (write(fd, buffer, strlen(buffer)) == -1) {
        write_log("[ERROR] Fallo al escribir encabezado en zombie.");
        close(fd);
        return;
    }

    time_t now = time(NULL);
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].is_active && process_table[i].state == ZOMBIE) {
            int diff_seconds = (int)difftime(now, process_table[i].start_time);
            int h = diff_seconds / 3600;
            int m = (diff_seconds % 3600) / 60;
            int s = diff_seconds % 60;

            snprintf(buffer, sizeof(buffer), "%-5d %-8d %-10s %02d:%02d:%02d %s\n",
                     process_table[i].iid,
                     (int)process_table[i].pid,
                     "ZOMBIE",
                     h, m, s,
                     process_table[i].binary_path);
            if (write(fd, buffer, strlen(buffer)) == -1) {
                write_log("[ERROR] Fallo al escribir fila en zombie.");
                break;
            }
        }
    }
    close(fd);
    write_log("Lista de procesos zombie enviada a res.txt");
}

int main(){
	signal(SIGPIPE, SIG_IGN);

	struct sigaction sa;
	sa.sa_handler = handle_sigchld;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGCHLD, &sa, NULL);

	int fd;
	char command[1100];
	init_daemon();
	create_and_open_fifo(&fd);
	while(1){
        int res = read_buffer(fd, command, sizeof(command));

        if(res < 0){
            if (errno == EINTR) {
                check_zombies();
            }
            continue;
        }

        check_zombies();

        if(res > 0){
            command[strcspn(command, "\r\n")] = 0;
            char log_msg[1200];
            snprintf(log_msg, sizeof(log_msg), "Comando recibido: '%s'", command);
            write_log(log_msg);
            char action[32] = {0};
            char arg[256] = {0};
            int num_args = sscanf(command, "%31s %255s", action, arg);

            if (num_args >= 1) {
                if (strcmp(action, "EXIT") == 0) {
                    write_log("Comando EXIT recibido. Terminando...");
                    break;
                }
                else if (strcmp(action, "start") == 0) {
                    if (num_args == 2) {
                        start(arg);
                    } else {
                        write_log("[ERROR] Sintaxis: start <ruta_binario>");
                    }
                }
                else if (strcmp(action, "stop") == 0) {
                    if (num_args == 2) {
                        int iid = atoi(arg);
                        stop(iid);
                    } else {
                        write_log("[ERROR] Sintaxis: stop <iid>");
                    }
                }
                else if (strcmp(action, "ps") == 0) {
                    show_processes();
                }
                else if (strcmp(action, "status") == 0) {
                    if (num_args == 2) {
                        int iid = atoi(arg);
                        show_status(iid);
                    } else {
                        write_log("[ERROR] Sintaxis: status <iid>");
                        show_status(-1);
                    }
                }
                else if (strcmp(action, "kill") == 0) {
                    if (num_args == 2) {
                        int iid = atoi(arg);
                        kill_process(iid);
                    } else {
                        write_log("[ERROR] Sintaxis: kill <iid>");
                    }
                }
                else if (strcmp(action, "zombie") == 0) {
                    show_zombies();
                }
                else {
                    write_log("Comando desconocido ignorado.");
                }
            }
        }
        else if(res == 0){
            write_log("Escritor cerro el pipe. Reabriendo FIFO...");
            close(fd);
            do {
                fd = open(FIFOPATH, O_RDONLY);
            } while (fd == -1 && errno == EINTR);

            if (fd == -1) {
                write_log("[ERROR FATAL] No se pudo reabrir el FIFO.");
                break;
            }
        }
    }
	close(fd);
	clean_fifo();
	return EXIT_SUCCESS;
}
