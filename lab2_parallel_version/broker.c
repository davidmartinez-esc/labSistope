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


int READ_END = 0;
int WRITE_END = 1;


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
    
    /*
    for (int y = 0; y < image.height; y++) {
        for (int x = 0; x < image.width; x++) {
            RGBPixel pixel = image.data[y * image.width + x];
            write(fd,&pixel,sizeof(RGBPixel));
        }
    }
    */
    // Y finalmente los datos de píxeles
    //write(fd, image->data, data_size);
    
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
   
    int i=0;

    int tuberias[2];

    int status=0;
        
    //DE AQUI EMPIEZA EL CODIGO IMPORTANTE
    //printf("  Ancho de la imagen: %d\n", image->width);
    //printf("  Alto de la imagen: %d\n", image->height);

    // Acceder a los píxeles de la imagen
    /*
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];
            printf("Pixel (%d, %d): R=%d, G=%d, B=%d\n", x, y, pixel.r, pixel.g, pixel.b);
        }
    }
    */
    pipe(tuberias);
  

    
    workers[0] = fork();
    if (workers[0] == 0) {
           close(tuberias[1]); // Cerramos el extremo de escritura del pipe en el hijo

        // Redirigir la entrada estándar para leer desde el pipe
           dup2(tuberias[READ_END], STDERR_FILENO);

           close(tuberias[READ_END]);
           //SOY EL HIJO
           

            char* argumentos[]={"./worker", "3", "4", NULL};
    // Para poder ejecutar debe existir el ejecutable
            execv(argumentos[0], argumentos);
           



        
             
    } else if (workers[0] < 0) {
            // Error al hacer fork
            perror("fork");
    } else {
        //SOY EL PADRE
            //char texto[100]="David y Claudio";
            const char* filename = N;
            BMPImage* image = read_bmp(filename);

            if (!image) {
                exit(1);
                return 1;
            }
                /*
                for (int y = 0; y < image->height; y++) {
                    for (int x = 0; x < image->width; x++) {
                        RGBPixel pixel = image->data[y * image->width + x];
                        printf("Pixel (%d, %d): R=%d, G=%d, B=%d\n", x, y, pixel.r, pixel.g, pixel.b);
                    }
                }
                */       
            int ancho= image->width;
           
            close(tuberias[READ_END]);


            write(tuberias[WRITE_END],&image->width,sizeof(int));
            write(tuberias[WRITE_END],&image->height,sizeof(int));
            //write(tuberias[1],texto,sizeof(char)*100);
            
            for (int y = 0; y < image->height; y++) {
                for (int x = 0; x < image->width; x++) {
                RGBPixel pixelBonito = image->data[y * image->width + x];
                int r=(int) pixelBonito.r;
                int g=(int) pixelBonito.g;
                int b=(int) pixelBonito.b;
                write(tuberias[WRITE_END],&pixelBonito.r,sizeof(unsigned char));
                write(tuberias[WRITE_END],&pixelBonito.g,sizeof(unsigned char));
                write(tuberias[WRITE_END],&pixelBonito.b,sizeof(unsigned char));
                }
            }
                
  
            printf("  CREADO WORKER 1 CON PID %d\n", workers[0]);
            printf("JUSTO ANTES DEL WRITE DEL PIPE DE LA IMAGEN \n");


  
            wait(&status);

            
            write_bmp("./PADREFORK.bmp",image);


            printf("ANTES DEL FREE \n");
            
            
            printf("  EL nombre del archivo es %s \n",N);
            printf("  Los valores ingresados son:\n NombreArchivo=%s\n numerodefiltros(f)=%i\n factor de saturacion(p)=%f \n Umbral para binarizar(u)=%f \n Umbral para clasificar(v)=%f \n NombreCarpeta(C)=%s \n NombreLogCsv(R)=%s \n numerodetrabajdores(W)=%i\n",N, f, p,u,v,C,R,W);
            printf("  Terminó el BROKER \n");

            free_bmp(image);
         
        }
    
 
    exit(0);
    return 0;
}
