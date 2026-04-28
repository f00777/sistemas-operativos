# toadd - Gestor de procesos tipo demonio

## Archivos

### toadd-daemon.c

Proceso que corre en segundo plano (demonio). Recibe comandos por un pipe FIFO (`commands.txt`) y gestiona una tabla interna de procesos hijos.

#### Funciones

- **write_log(info)**: Escribe un mensaje con timestamp y PID al archivo `toadd-daemon.log`.

- **add_process_to_table(pid, path)**: Busca un slot libre en la tabla de procesos y guarda el PID, la ruta del binario, el IID asignado y la hora de inicio. Si la tabla esta llena, registra un error en el log.

- **handle_sigchld(sig)**: Handler vacio para SIGCHLD. Su unico proposito es interrumpir llamadas bloqueantes como `read()` para que el demonio no se quede trabado cuando un hijo termina.

- **check_zombies()**: Usa `waitpid` con `WNOHANG` para recoger procesos hijos que terminaron. Si el proceso estaba en estado RUNNING, lo marca como ZOMBIE. Los que estaban en STOPPED no se tocan.

- **init_daemon()**: Hace el doble fork clasico para convertir el proceso en demonio. Crea una nueva sesion con `setsid`, cierra todos los file descriptors heredados, y redirige stdin/stdout/stderr a `/dev/null`.

- **init_process_table()**: Inicializa todos los slots de la tabla de procesos como inactivos.

- **clean_fifo()**: Si existe el archivo FIFO `commands.txt`, lo borra.

- **create_and_open_fifo(fd_ptr)**: Limpia cualquier FIFO anterior, crea uno nuevo con `mkfifo` y lo abre en modo lectura. Si el `open` se interrumpe por una señal, reintenta.

- **read_buffer(fd, dest, max_len)**: Lee datos del file descriptor dado y los guarda en `dest`. Retorna la cantidad de bytes leidos, 0 si el escritor cerro el pipe, o un valor negativo en caso de error.

- **start(binary_path)**: Primero verifica que el binario exista y sea ejecutable con `access`. Luego hace `fork`, y en el hijo llama a `setpgid(0,0)` para crear un grupo de procesos propio y despues `execlp` para ejecutar el binario. En el padre, agrega el proceso a la tabla.

- **stop(iid)**: Busca el proceso por IID en la tabla y le manda SIGTERM. Marca el estado como STOPPED.

- **kill_process(iid)**: Busca el proceso por IID y manda SIGKILL al grupo de procesos completo (`kill(-pid, SIGKILL)`), lo que mata al proceso y todos sus hijos. Luego lo recoge con `waitpid` y lo elimina de la tabla.

- **show_processes()**: Abre el FIFO de respuesta `res.txt` y escribe la tabla completa de procesos con formato de tabla (IID, PID, STATE, UPTIME, BINARY).

- **show_status(iid)**: Igual que `show_processes` pero solo escribe la fila del proceso que tiene el IID indicado. Si no existe, escribe un mensaje de error.

- **show_zombies()**: Igual que `show_processes` pero filtra solo los procesos en estado ZOMBIE.

- **main()**: Configura las señales (ignora SIGPIPE, registra handler para SIGCHLD sin SA_RESTART), se convierte en demonio, crea el FIFO y entra al loop principal donde lee comandos y los despacha.

---

### toadd-cli.c

Cliente de linea de comandos. Envia comandos al demonio por el pipe FIFO y, cuando corresponde, lee la respuesta.

#### Funciones

- **clean_fifo_res()**: Borra el archivo `res.txt` si existe.

- **main(argc, argv)**: Concatena los argumentos en un solo string de comando. Si el comando es `ps`, `status` o `zombie`, crea el FIFO `res.txt` antes de enviar para poder recibir la respuesta. Abre `commands.txt` en modo escritura, manda el comando, y si esperaba respuesta, abre `res.txt` en modo lectura e imprime lo que recibe.

---

## Comandos disponibles

| Comando | Descripcion |
|---------|-------------|
| `start <ruta>` | Ejecuta un binario y lo agrega a la tabla |
| `stop <iid>` | Envia SIGTERM al proceso (estado pasa a STOPPED) |
| `kill <iid>` | Envia SIGKILL al proceso y todos sus hijos, lo elimina de la tabla |
| `ps` | Muestra todos los procesos de la tabla |
| `status <iid>` | Muestra la info de un proceso especifico |
| `zombie` | Muestra solo los procesos en estado ZOMBIE |
| `EXIT` | Termina el demonio |

## Compilacion

```bash
gcc -o toadd-daemon toadd-daemon.c
gcc -o toadd-cli toadd-cli.c
```

## Uso

```bash
./toadd-daemon
./toadd-cli start ./mi_programa
./toadd-cli ps
./toadd-cli stop 2
./toadd-cli kill 3
./toadd-cli EXIT
```
