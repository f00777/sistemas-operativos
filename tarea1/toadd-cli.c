#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define FIFOPATH "commands.txt"
#define RESPATH "res.txt"
#define MAX_COMMAND_LEN 1100

void clean_fifo_res() {
    if (access(RESPATH, F_OK) == 0) {
        unlink(RESPATH);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <comando> [argumentos...]\n", argv[0]);
        fprintf(stderr, "Ejemplos:\n");
        fprintf(stderr, "  %s start ./rutabinario\n", argv[0]);
        fprintf(stderr, "  %s stop <iid>\n", argv[0]);
        fprintf(stderr, "  %s EXIT\n", argv[0]);
        return EXIT_FAILURE;
    }

    char command[MAX_COMMAND_LEN] = {0};
    int current_len = 0;

    for (int i = 1; i < argc; i++) {
        int len = strlen(argv[i]);
        if (current_len + len + 1 >= MAX_COMMAND_LEN) {
            fprintf(stderr, "Error: El comando es demasiado largo.\n");
            return EXIT_FAILURE;
        }
        if (i > 1) {
            strcat(command, " ");
            current_len++;
        }
        strcat(command, argv[i]);
        current_len += len;
    }

    int necesita_respuesta = (strcmp(command, "ps") == 0) ||
                             (strncmp(command, "status", 6) == 0) ||
                             (strcmp(command, "zombie") == 0);

    if (necesita_respuesta) {
        clean_fifo_res();
        if (mkfifo(RESPATH, 0666) == -1) {
            perror("Error al crear FIFO res.txt");
            return EXIT_FAILURE;
        }
    }

    int fd = open(FIFOPATH, O_WRONLY);
    if (fd == -1) {
        perror("Error al abrir commands.txt (el demonio no esta corriendo)");
        if (necesita_respuesta) clean_fifo_res();
        return EXIT_FAILURE;
    }

    if (write(fd, command, strlen(command)) == -1) {
        perror("Error al enviar el comando al demonio");
        close(fd);
        if (necesita_respuesta) clean_fifo_res();
        return EXIT_FAILURE;
    }

    close(fd);

    if (necesita_respuesta) {
        int res_fd = open(RESPATH, O_RDONLY);
        if (res_fd == -1) {
            perror("Error al abrir res.txt");
            clean_fifo_res();
            return EXIT_FAILURE;
        }

        char buffer[1024];
        ssize_t n;
        while ((n = read(res_fd, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[n] = '\0';
            printf("%s", buffer);
        }
        close(res_fd);
        clean_fifo_res();
    } else {
        printf("Comando encolado correctamente.\n");
    }

    return EXIT_SUCCESS;
}
