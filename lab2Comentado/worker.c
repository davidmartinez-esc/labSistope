#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include "funciones.h"
#include "fworker.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1

int main(int argc, char *argv[]) {
    // Descriptores de archivo para la lectura y escritura a través del pipe
    int fd[2];

    // Asignar descriptores de archivo a partir de los argumentos
    fd[READ_END]=atoi(argv[2]);
    fd[WRITE_END]=atoi(argv[1]);

    float saturationFactor=0.0;
    float binarizationFactor=0.0;

    BMPImage imagenRecibida;
 
    // Leer dimensiones de la imagen del pipe
    read(fd[READ_END],&imagenRecibida.width,sizeof(int));
    read(fd[READ_END],&imagenRecibida.height,sizeof(int));

    // Leer factores de saturación y binarización del pipe
    read(fd[READ_END],&saturationFactor,sizeof(float));
    read(fd[READ_END],&binarizationFactor,sizeof(float));

    // Reservar memoria para los datos de la imagen
    imagenRecibida.data = (RGBPixel*)malloc(imagenRecibida.width * imagenRecibida.height * sizeof(RGBPixel));

    // Leer los datos de píxeles de la imagen del pipe
    for (int y = 0; y < imagenRecibida.height; y++) {
        for (int x = 0; x < imagenRecibida.width; x++) {
            RGBPixel pixelRecibido;
            read(fd[READ_END],&pixelRecibido.r,sizeof(unsigned char));
            read(fd[READ_END],&pixelRecibido.g,sizeof(unsigned char));
            read(fd[READ_END],&pixelRecibido.b,sizeof(unsigned char));

            imagenRecibida.data[y * imagenRecibida.width + x] = pixelRecibido;
        }
    }

    // Aplicar saturación a la imagen recibida
    BMPImage* imagenSaturada=saturate_bmp(&imagenRecibida,saturationFactor);
   
    // Enviar la imagen saturada a través del pipe
    sendToBroker(imagenSaturada,fd);

    // Convertir la imagen saturada a escala de grises
    BMPImage* imagenGreyscale=greyscale_bmp(imagenSaturada);

    // Enviar la imagen en escala de grises a través del pipe
    sendToBroker(imagenGreyscale,fd);

    // Binarizar la imagen en escala de grises
    BMPImage* imagenBinarizada=binarize_bmp(imagenGreyscale,binarizationFactor);

    // Enviar la imagen binarizada a través del pipe
    sendToBroker(imagenBinarizada,fd);

    // Salir del proceso
    exit(0);

}
