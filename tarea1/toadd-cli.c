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
    // Verificamos que se haya pasado al menos un argumento
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

    // Concatenamos todos los argumentos en un solo string, separados por espacio
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

    int is_ps = (strcmp(command, "ps") == 0);

    if (is_ps) {
        clean_fifo_res();
        if (mkfifo(RESPATH, 0666) == -1) {
            perror("Error al crear FIFO res.txt");
            return EXIT_FAILURE;
        }
    }

    // Abrimos el FIFO en modo escritura
    int fd = open(FIFOPATH, O_WRONLY);
    if (fd == -1) {
        perror("Error al abrir commands.txt (¿Está corriendo el demonio?)");
        if (is_ps) clean_fifo_res();
        return EXIT_FAILURE;
    }

    // Escribimos el string completo en el FIFO
    if (write(fd, command, strlen(command)) == -1) {
        perror("Error al enviar el comando al daemon");
        close(fd);
        if (is_ps) clean_fifo_res();
        return EXIT_FAILURE;
    }

    close(fd);

    if (is_ps) {
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
