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
#define MAXBUFFERSIZE 128

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

            char bufferFDPadre[MAXBUFFERSIZE];
            char bufferFDHijo[MAXBUFFERSIZE];

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

//ESTO CREA UN ARRAY DE PIPES, LAS POSICIONES IMPARES CORRESPONDEN A UN PIPE DE COMUNICACION PADRE HIJO
//Y LOS PARES SON HIJOPADRE 
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


void send_image_through_pipe(int fd, BMPImage image) {
    printf("EMPEZÓ EL SEND IMAGE \n");
    write_bmp_nopointer("./DENTROSENDIMAGE.bmp",image);
    // Luego escribir la estructura BMPImage
    write(fd, &image, sizeof(BMPImage));
    
    printf("SE EJECUTÓ ENVIAR COSAS POR EL PIPE DE FORMA EFECTIVA\n");

    return;
}

BMPImage receive_image_from_pipe(int fd) {
  
    BMPImage image;
 

    // Leer la estructura BMPImage del pipe
    read(fd, &image, sizeof(BMPImage));
       
    printf("La imagen recibida tiene largo %d y alto %d ",image.width,image.height);
    printf("SE EJECUTÓ LEER COSAS POR EL PIPE DE FORMA EFECTIVA \n");
    write_bmp_nopointer("./DENTRORECEIVE.bmp",image);
    return image;
}



int main(int argc, char *argv[]) {
    printf("  Empezó el broker \n");
    char* N = argv[1];
    int f=atoi(argv[2]);
    float p=atof(argv[3]);
    float u=atof(argv[4]);
    float v=atof(argv[5]);
    int W=atoi(argv[6]);
    char C[100];
    char R[100];

    strcpy(C,argv[7]);
    strcpy(R,argv[8]);

    char pathSaturated[100]="./";
    char pathGreyScale[100]="./";
    char pathBinary[100]="./";

    char* image_names[] = {"saturated.bpm", "grey.bpm", "binary.bpm"};
    int classifications[] = {0, 0, 0};

    char bufferWidth[10];
    char bufferHeight[10];
    char bufferReadEnd[10];

    pid_t workers[W];

    int pids[W];
   
    int i=0;

    int tuberias[2];

    int**pipes;
    

    int status=0;
  
    pipes=crear_pipes(W);

    crear_workers(W,pipes,pids);

    int turno=0;

    const char* filename = N;
    BMPImage* image = read_bmp(filename);

    if (!image) {
                exit(1);
                return 1;
        }

    write_bmp("./juan.bmp",image);

    write(pipes[turno * 2 + 1][1],&image->width,sizeof(int));
    write(pipes[turno * 2 + 1][1],&image->height,sizeof(int));

            
    for (int y = 0; y < image->height; y++) {
                for (int x = 0; x < image->width; x++) {
                RGBPixel pixelBonito = image->data[y * image->width + x];
                int r=(int) pixelBonito.r;
                int g=(int) pixelBonito.g;
                int b=(int) pixelBonito.b;
                write(pipes[turno * 2 + 1][1],&pixelBonito.r,sizeof(unsigned char));
                write(pipes[turno * 2 + 1][1],&pixelBonito.g,sizeof(unsigned char));
                write(pipes[turno * 2 + 1][1],&pixelBonito.b,sizeof(unsigned char));
             }
    }

      write_bmp("./juanitu.bmp",image);

    for (int y = 0; y < image->height; y++) {
            for (int x = 0; x < image->width; x++) {
            RGBPixel pixelRecibido;
            read(pipes[turno * 2][0],&pixelRecibido.r,sizeof(unsigned char));
            read(pipes[turno * 2][0],&pixelRecibido.g,sizeof(unsigned char));
            read(pipes[turno * 2][0],&pixelRecibido.b,sizeof(unsigned char));
            //pixelRecibido.r=(unsigned char) r;
            //pixelRecibido.g=(unsigned char) g;
            //pixelRecibido.b=(unsigned char) b;

            image->data[y * image->width + x]= pixelRecibido;
            //imagenRecibida.data[y * imagenRecibida.width + x]=pixelRecibido;
            }
        }

       write_bmp("./gris.bmp",image);




            
                
      




     
  
        
 
    exit(0);
    return 0;
}
