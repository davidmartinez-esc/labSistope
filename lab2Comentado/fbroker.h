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

void crear_workers(int workers,int **pipes, int *pids);

// Entradas: El número de trabajadores (workers).
// Salidas: Retorna un puntero a un arreglo de pipes (int**).
// Descripción: Crea un número especificado de pipes para la comunicación entre el proceso padre y los procesos hijo.
// Cada trabajador tiene dos pipes asociados, uno para la lectura y otro para la escritura.
// En caso de error al crear un pipe, la función imprime un mensaje de error y termina el programa.

int **crear_pipes(int workers);