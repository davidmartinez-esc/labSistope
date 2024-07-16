//Ejemplo de lectura y escritura de imagen BMP en C
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "funciones.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

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

    int underscorePosition;
    char exampleFilename[10];
    char *underscorePtr;

    char pathSaturated[100]="./";
    char pathGreyScale[100]="./";
    char pathBinary[100]="./";
    
    int option;
    int mandatoryN=0;
    int mandatoryC=0;
    int mandatoryR=0;
    //Aqui definimos los nombres que tendrán las imagenes con los filtros aplicados, por ahora todas tendran el mismo nombre
    char* image_names[] = {"saturated.bpm", "grey.bpm", "binary.bpm"};
    int classifications[] = {0, 0, 0};
    //int num_images = sizeof(image_names) / sizeof(image_names[0]);

    while((option = getopt(argc, argv, "N:f:p:u:v:C:R:")) != -1){
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
    
    // Se lee la imagen que se desea procesar

    const char* filename = N;
    BMPImage* image = read_bmp(filename);
    if (!image) {
        return 1;
    }

    printf("Ancho de la imagen: %d\n", image->width);
    printf("Alto de la imagen: %d\n", image->height);

    // Acceder a los píxeles de la imagen
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];
            printf("Pixel (%d, %d): R=%d, G=%d, B=%d\n", x, y, pixel.r, pixel.g, pixel.b);
        }
    }
    
        mkdir(C,S_IRWXU);

        strcat(pathSaturated,C);
        strcat(pathSaturated,"/saturated.bmp");

        strcat(pathGreyScale,C);
        strcat(pathGreyScale,"/grey.bmp");

        strcat(pathBinary,C);
        strcat(pathBinary,"/binary.bmp");

        // 1.1f
        // Se crean todas las imagenes, independiente de lo que el usuario ingresó, pero solo se generan los archivos
        //de las imagenes que el usuario desea (por eso solo los write_bmp estan dentro de un if)
        //esto se hizo así ya que no se queria andar viendo que imagen se crea, cual imagen hay que liberar, cual imagen hay que escribir en el .csv y asi

        BMPImage* new_image = saturate_bmp(image,p);

        if(f>=1){
            write_bmp(pathSaturated, new_image);
        }
    
        BMPImage* grey_image = greyscale_bmp(new_image);

        if(f>=2){
            write_bmp(pathGreyScale, grey_image);
        }
        
        BMPImage* binary_image = binarize_bmp(grey_image,u);
        
        if(f>=3){
            write_bmp(pathBinary, binary_image);
        }
        
    // Se hace la clasificacion de cada imagen creada
    int isSaturatedNearly_black = is_nearly_black(new_image, v);
    int isGreyNearly_black = is_nearly_black(grey_image, v);
    int isBinaryNearly_black = is_nearly_black(binary_image, v);


    // Se asigna cada espacio del arreglo a su clasificacion si es casi negro
    classifications[0]=isSaturatedNearly_black;
    classifications[1]=isGreyNearly_black;
    classifications[2]=isBinaryNearly_black;
    
    //Dejamos esto por si la persona que revisa quiere testear la clasificacion d euna forma mas visual
    /*
    if (isSaturatedNearly_black==1) {
        printf("La imagen es casi negra.\n");
    } else {
        printf("La imagen no es casi negra.\n");
    }
    */

    //Esto tambien lo dejamos para que la persona que revisa lo descomente y pruebe para que vea los valores de las variables
    //printf("Los valores ingresados son:\n  PREFIJO=%s\n  numerodefiltros(f)=%i\n factor de saturacion(p)=%f \n Umbral para binarizar(u)=%f \n Umbral para clasificar(v)=%f \n C=%s \n R=%s \n",N, f, p,u,v,C,R);
    //printf("STRING %s Largo de la cadena %i \n",pathSaturated,(int)strlen(pathSaturated));

    // Se crea el archivo csv con las clasificaciones
    create_csv(R, image_names, classifications, f);

    // Se libera cada imagen
    free_bmp(new_image);
    
    free_bmp(grey_image);
  
    free_bmp(binary_image);
    
    free_bmp(image);

    
    return 0;
}
