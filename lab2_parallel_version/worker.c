//Ejemplo de lectura y escritura de imagen BMP en C
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

int main(int argc, char *argv[]) {


    int fd[2];

    fd[READ_END]=atoi(argv[2]);
    fd[WRITE_END]=atoi(argv[1]);

    float saturationFactor=0.0;
    float binarizationFactor=0.0;

    BMPImage imagenRecibida;
 
    read(fd[READ_END],&imagenRecibida.width,sizeof(int));
    read(fd[READ_END],&imagenRecibida.height,sizeof(int));

    read(fd[READ_END],&saturationFactor,sizeof(float));
    read(fd[READ_END],&binarizationFactor,sizeof(float));

    imagenRecibida.data = (RGBPixel*)malloc(imagenRecibida.width * imagenRecibida.height * sizeof(RGBPixel));

  

     for (int y = 0; y < imagenRecibida.height; y++) {
            for (int x = 0; x < imagenRecibida.width; x++) {
            RGBPixel pixelRecibido;
            read(fd[READ_END],&pixelRecibido.r,sizeof(unsigned char));
            read(fd[READ_END],&pixelRecibido.g,sizeof(unsigned char));
            read(fd[READ_END],&pixelRecibido.b,sizeof(unsigned char));

            imagenRecibida.data[y * imagenRecibida.width + x] = pixelRecibido;
            }
        }

      

      
   
    BMPImage* imagenSaturada=saturate_bmp(&imagenRecibida,saturationFactor);
   
    sendToBroker(imagenSaturada,fd);

    BMPImage* imagenGreyscale=greyscale_bmp(imagenSaturada);

    sendToBroker(imagenGreyscale,fd);

    BMPImage* imagenBinarizada=binarize_bmp(imagenGreyscale,binarizationFactor);

    sendToBroker(imagenBinarizada,fd);


    exit(0);

}
