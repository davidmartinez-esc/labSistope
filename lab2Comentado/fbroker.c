#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "funciones.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

// Entradas: El número de trabajadores (workers), un arreglo de pipes (pipes) y un arreglo de pids (pids).
// Salidas: void
// Descripción: Crea un número especificado de procesos hijo (trabajadores) y configura la comunicación entre el padre e hijos usando pipes.
// Cada hijo ejecuta un programa llamado "worker" con los descriptores de archivo de lectura y escritura pasados como argumentos.
// El padre cierra los extremos no utilizados de los pipes y almacena los pids de los procesos hijo.

void crear_workers(int workers,int **pipes, int *pids)
{
    for (int i = 0; i < workers; i++) {
        int pid = fork();
        if (pid < 0) {
            printf("Error al crear el proceso hijo\n");
            exit(1);
        } else if (pid == 0) {
            // Proceso hijo ejecuta worker
            close(pipes[2 * i][0]);
            close(pipes[2 * i + 1][1]);

            char bufferFDPadre[128];
            char bufferFDHijo[128];

            sprintf(bufferFDPadre, "%d", pipes[2 * i][1]); //ESTA ES LA QUE EL HIJO USA PARA ESCRIBIRLE AL PADRE
            sprintf(bufferFDHijo, "%d", pipes[2 * i + 1][0]); //DE ESTA TIENE QUE LEER EL HIJO


            char* argv_exec[] = {"./worker", bufferFDPadre,
                                 bufferFDHijo, NULL};
            execv("./worker", argv_exec);
        } else {
            // Proceso padre
            pids[i] = pid; // almacenar pid
            close(pipes[i * 2][1]);
            close(pipes[i * 2 + 1][0]);
        }
    }
}

// Entradas: El número de trabajadores (workers).
// Salidas: Retorna un puntero a un arreglo de pipes (int**).
// Descripción: Crea un número especificado de pipes para la comunicación entre el proceso padre y los procesos hijo.
// Cada trabajador tiene dos pipes asociados, uno para la lectura y otro para la escritura.
// En caso de error al crear un pipe, la función imprime un mensaje de error y termina el programa.

int **crear_pipes(int workers)
{
    int **pipes;
    pipes = (int**)malloc(workers * 2 * sizeof(int*));
    for (int i = 0; i < workers * 2; i++) {
        pipes[i] = (int*)malloc(2 * sizeof(int));
    }

    for (int i = 0; i < workers * 2; i++) { // 2 por cada worker
        if (pipe(pipes[i]) == -1)
        {
            printf("Error al crear el pipe\n");
            exit(1);
        }
    }
    return pipes;
}
