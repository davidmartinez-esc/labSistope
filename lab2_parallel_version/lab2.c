//Ejemplo de lectura y escritura de imagen BMP en C
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "funciones.h"

#include <sys/wait.h>

/*
LABORATORIO DESARROLLADO POR 

CLAUDIO MATIAS PALMA PEREZ SECCION ACOSTA 20.680.606-0
DAVID ENRIQUE MARTINEZ ESCARES 20.882.683-2

CONSIDERACIONES:

Cuando se genera la carpeta con las imagenes no se genera de inmediato las imagenes
debe entrar y salir varias veces de la carpeta que se genera para que se muestren las que Ud. generó

Laboratorio basado en el aporte de codigo que mandó el ayudante Ian Rickmers

Laboratorio testeado en Ubuntu 22.04.4 LTS
*/

int main(int argc, char *argv[]) {
    // Se crean las variables para el getopt, la clasificacion de las imagenes y la creacion de estas
    char N[100];
    char R[100];
    int f=3;
    float p=1.3;
    float u=0.5;
    float v=0.5;
    char C[100];
    pid_t pid;
    int W=1;

    int option;
    int mandatoryN=0;
    int mandatoryC=0;
    int mandatoryR=0;

    int underscorePosition;
    char exampleFilename[10];
    char *underscorePtr;
    //Aqui definimos los nombres que tendrán las imagenes con los filtros aplicados, por ahora todas tendran el mismo nombre
    
    //int num_images = sizeof(image_names) / sizeof(image_names[0]);

    char bufferF[10];
    char bufferP[10]; 
    char bufferU[10]; 
    char bufferV[10];
    char bufferW[10];   

    while((option = getopt(argc, argv, "N:f:p:u:v:C:R:W:")) != -1){
        switch(option){
            case 'N':
                strcpy(N, optarg);
                
                mandatoryN=1;
                
                break;
            case 'f':
                f = atoi(optarg);
                break;
            case 'p':
                p = atof(optarg);

                break;
            case 'u':
                u = atof(optarg);
                break;
            case 'v':
                v = atof(optarg);
                break;
            case 'C':
                strcpy(C,optarg);
                mandatoryC=1;
                break;
            case 'R':
                strcpy(R,optarg);
                strcat(R,".csv");
                mandatoryR=1;
                break;
            case 'W':
                W= atoi(optarg);
                break;
        }
    }
    
    if(f<=0 || f>3){
        printf("La flag f solo puede tomar los valores 1,2,3 \n");
        return 1;
    }

    if(p<=0){
        printf("La flag p no puede tomar el valor 0 o menor que este");
        return 1;
    }

    if(u<0 || u>1){
         printf("La flag u puede tomar valores entre 0 y 1 (incluyendolos)");
        return 1;
    }

     if(v<0 || v>1){
         printf("La flag v puede tomar valores entre 0 y 1 (incluyendolos)");
        return 1;
    }

    if(mandatoryN == 0){
        printf("Se debe ingresar el nombre del prefijo de la imagen (img, imagen y photo)\n");
        return 1;
    }
    
    if(mandatoryC == 0){
        printf("Se debe ingresar el nombre de la carpeta con los archivos resultantes \n");
        return 1;
    }

    if(mandatoryR == 0){
        printf("Se debe ingresar el nombre del archivo .csv con las clasificaciones \n");
        return 1;
    }
    
    /*
    Lo de aqui abajo es para que siga el formato imagen_N, photo_N, img_N
    Tambien dejamos habilitado que pueda ponerse imagen, img, photo sin el _N al final, ya que en el ejemplo
    del pdf III.H.Línea de comando el ejemplo que dan es -N imagen sin nada mas
    */
    strncpy(exampleFilename,N,4);    
    if(strcmp(exampleFilename,"img")!=0){
        strncpy(exampleFilename,N,6);

        if(strcmp(exampleFilename,"photo")!=0){
            strncpy(exampleFilename,N,7);

            if(strcmp(exampleFilename,"imagen")!=0){
                underscorePtr = strchr(N, '_');

                if(underscorePtr == NULL)
                {
                    printf("El string no tiene el caracter _ \n");
                    return 1;
                }
                underscorePosition=underscorePtr-N;
                if(underscorePosition!=3 && underscorePosition!=5 && underscorePosition!=6){

                    printf("El nombre que usted ingresó no sigue el formato que usa de prefijo image, img o photo  \n");
                    return 1;
                }
                if(atoi(underscorePtr+1)==0){
                    printf("El nombre de archivo que usted ingresó no sigue el formato image_N, photo_N o img_N siendo N un entero \n");
                    return 1;
                }

            }
        }

    }

    //Aqui le pongo la extension al nombre del archivo ingresado con la flag -N
    strcat(N,".bmp");

    pid = fork();

    if(pid == 0){
        

        sprintf(bufferF, "%d", f);
        sprintf(bufferP, "%f", p);
        sprintf(bufferU, "%f", u);
        sprintf(bufferV, "%f", v);
        sprintf(bufferW, "%i", W);

         // Se lee la imagen que se desea procesar
        char* argv[]={"./broker", N, bufferF,bufferP,bufferU,bufferV,bufferW,C,R,NULL};
         // Para poder ejecutar debe existir el ejecutable
        execv(argv[0], argv);
    }
    // Si el pid es > a 0 significa que es el proceso padre
    else{
        
        waitpid(pid, NULL, 0);

    }
    

    printf("Terminó el MAIN \n");
    return 0;
}
