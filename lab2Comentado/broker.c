#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include "funciones.h"
#include "fbroker.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1
#define MAXBUFFERSIZE 128

int main(int argc, char *argv[]) {
    // Argumentos del programa
    // N: nombre del archivo BMP a leer
    // f: entero convertido de argv[2]
    // p: flotante convertido de argv[3]
    // u: flotante convertido de argv[4]
    // v: flotante convertido de argv[5]
    // W: número de trabajadores (workers) convertido de argv[6]
    // C: nombre del directorio de salida copiado de argv[7]
    // R: nombre de otro directorio copiado de argv[8]
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

  

   

    int pids[W];
   
    int fragmentsCollected=0;
    int fragmentsTransfered=0;

    // Crear pipes para la comunicación entre el padre y los trabajadores
    int**pipes;
    
    pipes=crear_pipes(W);

    crear_workers(W,pipes,pids);

    

    const char* filename = N;
    BMPImage* image = read_bmp(filename);

    // Verificar si la imagen fue leída correctamente
    if (!image) {
                exit(1);
                return 1;
        }
    // Calcular el ancho de cada fragmento a procesar por cada trabajador

    int fragmentWidth=image->width / W;
    int resto=image->width%W;

    int inicio=0;
    int fin=fragmentWidth;

    fragmentsTransfered=0;

    // Crear el directorio de salida
    mkdir(C,S_IRWXU);
    // Configurar las rutas de salida para las imágenes procesadas
    strcat(pathSaturated,C);
    strcat(pathSaturated,"/saturated.bmp");

    strcat(pathGreyScale,C);
    strcat(pathGreyScale,"/grey.bmp");

    strcat(pathBinary,C);
    strcat(pathBinary,"/binary.bmp");
    
    // Enviar los fragmentos de la imagen a los trabajadores
    while(fragmentsTransfered!=W){

        write(pipes[fragmentsTransfered * 2 + 1][1],&fragmentWidth,sizeof(int));
        write(pipes[fragmentsTransfered * 2 + 1][1],&image->height,sizeof(int));

        write(pipes[fragmentsTransfered * 2 + 1][1],&p,sizeof(float));
        write(pipes[fragmentsTransfered * 2 + 1][1],&u,sizeof(float));

        for (int y = 0; y < image->height; y++) {
            for (int x=inicio; x < fin; x++) {
                RGBPixel pixelBonito = image->data[y * image->width + x];
               
                write(pipes[fragmentsTransfered * 2 + 1][1],&pixelBonito.r,sizeof(unsigned char));
                write(pipes[fragmentsTransfered * 2 + 1][1],&pixelBonito.g,sizeof(unsigned char));
                write(pipes[fragmentsTransfered * 2 + 1][1],&pixelBonito.b,sizeof(unsigned char));
            }
        }
        inicio=inicio+fragmentWidth;
        if(fragmentsTransfered==(W-1)){
            fin=fin+fragmentWidth+resto;
        }
        else{
            fin=fin+fragmentWidth;
        }
        fragmentsTransfered++;
    }

    //RESETEAMOS INICIO Y FIN
    //Este proceso se repite 3 veces para recibir las 3 imagenes.
    //No fue pasado a funcion debido a que dejaba de funcionar.

    inicio=0;
    fin=fragmentWidth;

    fragmentsCollected=0;
    
    while(fragmentsCollected!=W){
     
        for (int y = 0; y < image->height; y++) {
            for (int x=inicio; x < fin; x++) {
            RGBPixel pixelRecibido;
            read(pipes[fragmentsCollected * 2][0],&pixelRecibido.r,sizeof(unsigned char));
            read(pipes[fragmentsCollected * 2][0],&pixelRecibido.g,sizeof(unsigned char));
            read(pipes[fragmentsCollected * 2][0],&pixelRecibido.b,sizeof(unsigned char));

            image->data[y * image->width + x]= pixelRecibido;
            //imagenRecibida.data[y * imagenRecibida.width + x]=pixelRecibido;
            }
        }
        inicio=inicio+fragmentWidth;
        
         if(fragmentsCollected==(W-1)){
        fin=fin+fragmentWidth+resto;
        }
        else{
            fin=fin+fragmentWidth;
        }
        fragmentsCollected++;
    }
    if(f>=1){
        write_bmp(pathSaturated,image);
    }
    int isSaturatedNearly_black = is_nearly_black(image, v);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    inicio=0;
    fin=fragmentWidth;

    fragmentsCollected=0;
    
    while(fragmentsCollected!=W){
     
        for (int y = 0; y < image->height; y++) {
            for (int x=inicio; x < fin; x++) {
            RGBPixel pixelRecibido;
            read(pipes[fragmentsCollected * 2][0],&pixelRecibido.r,sizeof(unsigned char));
            read(pipes[fragmentsCollected * 2][0],&pixelRecibido.g,sizeof(unsigned char));
            read(pipes[fragmentsCollected * 2][0],&pixelRecibido.b,sizeof(unsigned char));

            image->data[y * image->width + x]= pixelRecibido;
            //imagenRecibida.data[y * imagenRecibida.width + x]=pixelRecibido;
            }
        }
        inicio=inicio+fragmentWidth;
        
         if(fragmentsCollected==(W-1)){
            fin=fin+fragmentWidth+resto;
        }
        else{
            fin=fin+fragmentWidth;
        }
        

        fragmentsCollected++;

    }
    if(f>=2){
        write_bmp(pathGreyScale,image);
    }
    int isGreyNearly_black = is_nearly_black(image, v);
    /////////////////////////////////////////////////////////////////////////////////////////////////

    inicio=0;
    fin=fragmentWidth;

    fragmentsCollected=0;
    
    while(fragmentsCollected!=W){
     
        for (int y = 0; y < image->height; y++) {
            for (int x=inicio; x < fin; x++) {
                RGBPixel pixelRecibido;
                read(pipes[fragmentsCollected * 2][0],&pixelRecibido.r,sizeof(unsigned char));
                read(pipes[fragmentsCollected * 2][0],&pixelRecibido.g,sizeof(unsigned char));
                read(pipes[fragmentsCollected * 2][0],&pixelRecibido.b,sizeof(unsigned char));

                image->data[y * image->width + x]= pixelRecibido;
            //imagenRecibida.data[y * imagenRecibida.width + x]=pixelRecibido;
            }
        }
        inicio=inicio+fragmentWidth;
        
        if(fragmentsCollected==(W-1)){
            fin=fin+fragmentWidth+resto;
        }
        else{
            fin=fin+fragmentWidth;
        }
        fragmentsCollected++;
    }
    if(f>=3){
        write_bmp(pathBinary,image);
    }

    int isBinaryNearly_black = is_nearly_black(image, v);
   
    classifications[0]=isSaturatedNearly_black;
    classifications[1]=isGreyNearly_black;
    classifications[2]=isBinaryNearly_black;

    create_csv(R, image_names, classifications, f);

            
    exit(0);
    return 0;
}
