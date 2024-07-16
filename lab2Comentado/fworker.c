#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "funciones.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1

// Entradas: Un puntero a una imagen BMP (BMPImage* imagen) y un arreglo de descriptores de archivo (int* fd).
// Salidas: void
// Descripción: Envía los datos de píxeles de una imagen BMP a través de un pipe.
// Recorre cada píxel de la imagen, obteniendo los valores de rojo (r), verde (g) y azul (b) y los escribe en el pipe especificado.

void sendToBroker(BMPImage* imagen, int*fd){
     for (int y = 0; y < imagen->height; y++) {
                for (int x = 0; x < imagen->width; x++) {
                RGBPixel pixelBonito = imagen->data[y * imagen->width + x];
             
                write(fd[WRITE_END],&pixelBonito.r,sizeof(unsigned char));
                write(fd[WRITE_END],&pixelBonito.g,sizeof(unsigned char));
                write(fd[WRITE_END],&pixelBonito.b,sizeof(unsigned char));
             }
    }  
}