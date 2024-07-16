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

void write_bmp_nopointer(const char* filename, BMPImage image) {
    FILE* file = fopen(filename, "wb"); //wb = write binary
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo.\n");
        return;
    }

    BMPHeader header;
    header.type = 0x4D42;
    header.size = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + image.width * image.height * sizeof(RGBPixel);
    header.offset = sizeof(BMPHeader) + sizeof(BMPInfoHeader);

    BMPInfoHeader info_header;
    info_header.size = sizeof(BMPInfoHeader);
    info_header.width = image.width;
    info_header.height = image.height;
    info_header.planes = 1;
    info_header.bit_count = 24; // está fijado en 24 en este ejemplo pero puede ser 1, 4, 8, 16, 24 o 32
    info_header.size_image = image.width * image.height * sizeof(RGBPixel);

    fwrite(&header, sizeof(BMPHeader), 1, file);
    fwrite(&info_header, sizeof(BMPInfoHeader), 1, file);

    int padding = (4 - (image.width * sizeof(RGBPixel)) % 4) % 4;
    for (int y = image.height - 1; y >= 0; y--) {
        for (int x = 0; x < image.width; x++) {
            RGBPixel pixel = image.data[y * image.width + x];
            fwrite(&pixel, sizeof(RGBPixel), 1, file);
        }

        RGBPixel padding_pixel = {0};
        fwrite(&padding_pixel, sizeof(RGBPixel), padding, file);
    }

    fclose(file);
}



BMPImage* receive_image_from_pipe(int fd) {
    size_t image_size;
    BMPImage *image;
    size_t data_size;

    // Leer el tamaño total de los datos que se van a recibir
    if (read(fd, &image_size, sizeof(size_t)) < sizeof(size_t)) {
        perror("Error al leer el tamaño de la imagen desde el pipe");
        return NULL;
    }

    // Verificar que el tamaño sea válido
    if (image_size < sizeof(BMPImage)) {
        fprintf(stderr, "Tamaño de imagen recibido inválido.\n");
        return NULL;
    }

    // Almacenar el tamaño de los datos de píxeles
    data_size = image_size - sizeof(BMPImage);

    // Asignar memoria para la imagen
    image = (BMPImage*)malloc(sizeof(BMPImage));
    if (!image) {
        perror("Error al asignar memoria para la imagen BMP");
        return NULL;
    }

    // Leer la estructura BMPImage del pipe
    if (read(fd, image, sizeof(BMPImage)) < sizeof(BMPImage)) {
        perror("Error al leer la estructura BMPImage desde el pipe");
        free(image);
        return NULL;
    }

    image->width=640;
    image->height=426;
    

    // Asignar memoria para los datos de píxeles
    image->data = (RGBPixel*)malloc(sizeof(RGBPixel) * image->width * image->height);

    if (!image->data) {
        perror("Error al asignar memoria para los datos de píxeles");
        free(image);
        return NULL;
    }


    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel;
            read(fd, &pixel, sizeof(RGBPixel));
            
            image->data[y * image->width + x] = pixel;
        }
    }
    

    // Leer los datos de píxeles del pipe
    /*
    if (read(fd, image->data, data_size) < data_size) {
        perror("Error al leer los datos de píxeles desde el pipe");
        free(image->data);
        free(image);
        return NULL;
    }
    */
    printf("SE EJECUTÓ LEER COSAS POR EL PIPE DE FORMA EFECTIVA \n");

    return image;
}

int main(int argc, char *argv[]) {
    printf("    Empezó el WORKER \n");

    int fd[2];

    fd[READ_END]=atoi(argv[2]);
    fd[WRITE_END]=atoi(argv[1]);

    BMPImage imagenRecibida;
 
    read(fd[READ_END],&imagenRecibida.width,sizeof(int));
    read(fd[READ_END],&imagenRecibida.height,sizeof(int));

    imagenRecibida.data = (RGBPixel*)malloc(imagenRecibida.width * imagenRecibida.height * sizeof(RGBPixel));

    int r;
    int g;
    int b;
    printf("EMPEZÓ A LEER  WORKER\n");
     for (int y = 0; y < imagenRecibida.height; y++) {
            for (int x = 0; x < imagenRecibida.width; x++) {
            RGBPixel pixelRecibido;
            read(fd[READ_END],&pixelRecibido.r,sizeof(unsigned char));
            read(fd[READ_END],&pixelRecibido.g,sizeof(unsigned char));
            read(fd[READ_END],&pixelRecibido.b,sizeof(unsigned char));
            //pixelRecibido.r=(unsigned char) r;
            //pixelRecibido.g=(unsigned char) g;
            //pixelRecibido.b=(unsigned char) b;

            imagenRecibida.data[y * imagenRecibida.width + x] = pixelRecibido;
            //imagenRecibida.data[y * imagenRecibida.width + x]=pixelRecibido;
            }
        }

        printf("TERMINÓ DE LEER WORKER \n");

      
   
    BMPImage* imagenEscalaGrises=greyscale_bmp(&imagenRecibida);
   

    for (int y = 0; y < imagenEscalaGrises->height; y++) {
                for (int x = 0; x < imagenEscalaGrises->width; x++) {
                RGBPixel pixelBonito = imagenEscalaGrises->data[y * imagenEscalaGrises->width + x];
                int r=(int) pixelBonito.r;
                int g=(int) pixelBonito.g;
                int b=(int) pixelBonito.b;
                write(fd[WRITE_END],&pixelBonito.r,sizeof(unsigned char));
                write(fd[WRITE_END],&pixelBonito.g,sizeof(unsigned char));
                write(fd[WRITE_END],&pixelBonito.b,sizeof(unsigned char));
             }
    }
    write_bmp("./creadaPorWorker.bmp",imagenEscalaGrises);

    printf("terminó WORKER \n");
    exit(0);

}
