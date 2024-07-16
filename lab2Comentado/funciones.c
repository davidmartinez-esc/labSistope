//Ejemplo de lectura y escritura de imagen BMP en C
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "funciones.h"

// Entradas: Recibe un archivo con extensión.
// Salidas: Retorna un puntero al archivo de imagen tipo BMP con el contenido que leyó del archivo. En caso contrario retorna NULL.
// Descripción: Esta función lee una imagen BMP desde un archivo especificado y la carga en memoria. 
// Primero intenta abrir el archivo en modo de lectura binaria ("rb"), si no puede abrir el archivo, imprime un mensaje de error y retorna NULL.
// Luego, lee la cabecera BMP y verifica si es un archivo BMP válido comprobando el campo de tipo de la cabecera.
// Si la cabecera no indica que es un archivo BMP válido, imprime un mensaje de error, cierra el archivo y retorna NULL.
// Después de leer la cabecera, lee la cabecera de información BMP para obtener detalles como el ancho, alto y el desplazamiento a los datos de imagen.
// Luego, crea una estructura BMPImage y reserva memoria para almacenar los datos de la imagen.
// Luego, se desplaza a la posición indicada por el desplazamiento en la cabecera BMP para comenzar a leer los datos de la imagen.
// Durante la lectura de los datos de la imagen, se aplica un relleno si es necesario para asegurar que el ancho de la imagen sea múltiplo de 4 bytes.
// Los datos de la imagen se leen en una estructura RGBPixel y se almacenan en el arreglo de datos de la imagen.
// Una vez que se han leído todos los datos de la imagen, se cierra el archivo y se retorna el puntero a la estructura BMPImage que contiene la imagen leída.

BMPImage* read_bmp(const char* filename) {
    FILE* file = fopen(filename, "rb"); //rb = read binary
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo.\n");
        return NULL;
    }

    BMPHeader header;
    fread(&header, sizeof(BMPHeader), 1, file);
    if (header.type != 0x4D42) { // 42 = 66 en decimal = B en ASCII y 4D = 77 en decimal = M en ASCII
        fprintf(stderr, "Error: El archivo no es un BMP válido.\n");
        fclose(file);
        return NULL;
    }

    BMPInfoHeader info_header;
    fread(&info_header, sizeof(BMPInfoHeader), 1, file); //fread(puntero a la estructura, tamaño de la estructura, cantidad de estructuras, archivo)

    BMPImage* image = (BMPImage*)malloc(sizeof(BMPImage));
    image->width = info_header.width;
    image->height = info_header.height;
    image->data = (RGBPixel*)malloc(sizeof(RGBPixel) * info_header.width * info_header.height);

    fseek(file, header.offset, SEEK_SET); // fseek(archivo, desplazamiento, origen desde donde se desplaza SEEK_SET = inicio del archivo, SEEK_CUR = posición actual del archivo, SEEK_END = final del archivo)

    //se hace padding para que la imagen tenga un tamaño múltiplo de 4, esto se hace para que la imagen sea más rápida de leer
    int padding = (4 - (info_header.width * sizeof(RGBPixel)) % 4) % 4; // primero se pasan a bytes los píxeles de la imagen y se calcula el residuo de la división entre 4, si el residuo es 0 no hay padding, si el residuo es 1, 2 o 3 se calcula el padding
    for (int y = info_header.height - 1; y >= 0; y--) {
        for (int x = 0; x < info_header.width; x++) {
            RGBPixel pixel;
            fread(&pixel, sizeof(RGBPixel), 1, file);
            image->data[y * info_header.width + x] = pixel;
        }
        fseek(file, padding, SEEK_CUR);
    }

    fclose(file);
    return image;
}

// Entradas: El nombre del archivo a crear y el puntero a la imagen creada.
// Salidas: void, solo crea el archivo resultante.
// Descripción: Guarda la imagen leida en un archivo con el nombre indicado por parametros. 
// En primer lugar se encarga de abrir el archivo en modo escritura binaria ("wb"), en caso de que no pueda abrirlo lo cierra y retorna NULL.
// Escribe la cabecera de la imagen BMP, donde se encuentra la información respecto a su ancho, alto, cantidad de color y profundidad de bits.
// Luego empieza a escribir los datos de la image, realizandolo pixel por pixel comenzando desde la esquina izquierda y aplica relleno en caso de ser necesario
// para que sea multiplo de 4 bytes. Una vez termina se cierra la función y crea el archivo. 

void write_bmp(const char* filename, BMPImage* image) {
    FILE* file = fopen(filename, "wb"); //wb = write binary
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo.\n");
        return;
    }

    BMPHeader header;
    header.type = 0x4D42;
    header.size = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + image->width * image->height * sizeof(RGBPixel);
    header.offset = sizeof(BMPHeader) + sizeof(BMPInfoHeader);

    BMPInfoHeader info_header;
    info_header.size = sizeof(BMPInfoHeader);
    info_header.width = image->width;
    info_header.height = image->height;
    info_header.planes = 1;
    info_header.bit_count = 24; // está fijado en 24 en este ejemplo pero puede ser 1, 4, 8, 16, 24 o 32
    info_header.size_image = image->width * image->height * sizeof(RGBPixel);

    fwrite(&header, sizeof(BMPHeader), 1, file);
    fwrite(&info_header, sizeof(BMPInfoHeader), 1, file);

    int padding = (4 - (image->width * sizeof(RGBPixel)) % 4) % 4;
    for (int y = image->height - 1; y >= 0; y--) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];
            fwrite(&pixel, sizeof(RGBPixel), 1, file);
        }

        RGBPixel padding_pixel = {0};
        fwrite(&padding_pixel, sizeof(RGBPixel), padding, file);
    }

    fclose(file);
}

// Entradas: Un puntero a una imagen creada.
// Salidas: void
// Descripción: Esta funcion se encarga de liberar la memoria utilizada por la imagen al momento de su lectura y manipulación.

void free_bmp(BMPImage* image) {
    if (image) {
        free(image->data);
        free(image);
    }
}

// Entradas: Un puntero a imagen que es la imagen que se desea saturar y un factor de punto flotante que indica la cantidad de saturación que se desea aplicar a cada pixel. 
// Salidas: Un puntero a imagen con la imagen ya saturada según el factor entregado.
// Descripción: La función se encarga de guardar memoria para la nueva imagen binarizada saturada, posteriormente guarda memoria para un pixel de la imagen para ir alterandolos 
// posteriormente de forma que cada pixel es separado segun rojo, azul y verde. 
// Luego la imagen es recorrida y cada pixel RGB es multiplicado por el factor ingresado. 
// Una vez hecho esto se le asigna a la imagen nueva los pixeles nuevos y se retorna.


BMPImage* saturate_bmp(BMPImage* image, float factor) {
    BMPImage* new_image = (BMPImage*)malloc(sizeof(BMPImage));
    new_image->width = image->width;
    new_image->height = image->height;
    new_image->data = (RGBPixel*)malloc(sizeof(RGBPixel) * image->width * image->height);

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];
            pixel.r = (unsigned char)(pixel.r * factor);
            if(pixel.r>255){
                pixel.r=(unsigned char) 255;
            }
            pixel.g = (unsigned char)(pixel.g * factor);
            if(pixel.g > 255){
                pixel.g = 255;
            }
            pixel.b = (unsigned char)(pixel.b * factor);
            if(pixel.b > 255){
                pixel.b = 255;
            }
            new_image->data[y * image->width + x] = pixel;
        }
    }

    return new_image;
}

// Entradas: Un puntero a imagen que es la imagen a la que se desea aplicar escala de grises.
// Salidas: Un puntero a imagen con la imagen en escala de grises.
// Descripción: Caso similar a la función de saturar, se guarda espacio para la nueva imagen binarizada que se va a crear así como también para un pixel que represente el RGB de la imagen,
// solo que en este caso se crea una nueva variable donde cada pixel de la imagen, que fue recorrida mediante dos ciclos, es multiplicado por valores fijos que producen
// escala de grises, en este caso siendo pixel.red * 0.3, pixel.blue * 0.11 y pixel.green * 0.59.
// Una vez realizado todo este procedimiento se asignan los valores grises a cada pixel y luego se guardan en la nueva imagen binarizada y es retornada al usuario.

BMPImage* greyscale_bmp(BMPImage* image) {
    BMPImage* new_image = (BMPImage*)malloc(sizeof(BMPImage));
    new_image->width = image->width;
    new_image->height = image->height;
    new_image->data = (RGBPixel*)malloc(sizeof(RGBPixel) * image->width * image->height);

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];
            unsigned char gray = (unsigned char)(0.3 * pixel.r + 0.59 * pixel.g + 0.11 * pixel.b);
            pixel.r = pixel.g = pixel.b = gray;
            new_image->data[y * image->width + x] = pixel;
        }
    }
    return new_image;
}

// Entradas: Un puntero a imagen que es la imagen que se desea saturar y un factor de punto flotante que indica el umbral al que se compara cada pixel, debe ser entre 0 y 1. 
// Salidas: Un puntero a imagen con la imagen ya binarizada.
// Descripción: Esta función binariza una imagen en escala de grises basada en un umbral especificado. 
// Primero, convierte la imagen original a escala de grises utilizando la función grayscale_bmp para simplificar el proceso de binarización
// y mejorar la consistencia en los resultados. 
// Luego, crea una nueva estructura BMPImage para almacenar la imagen binarizada y reserva memoria para los datos de la imagen. 
// Para binarizar la imagen se itera sobre cada píxel de la imagen en escala de grises y se compara su valor de gris con el umbral. 
// Si el valor de gris del píxel es mayor que el umbral, se asigna blanco al píxel en la imagen binarizada, en caso contrario (menor o igual al umbral), 
// se asigna negro al píxel en la imagen binarizada. 
// Una vez que se ha binarizado toda la imagen, se libera la memoria de la imagen en escala de grises y se retorna la imagen binarizada.

BMPImage* binarize_bmp(BMPImage* grayscale_image, float threshold) {
    //BMPImage* grayscale_image = greyscale_bmp(image); Convertir la imagen a escala de grises

    BMPImage* binarized_image = (BMPImage*)malloc(sizeof(BMPImage));
    binarized_image->width = grayscale_image->width;
    binarized_image->height = grayscale_image->height;
    binarized_image->data = (RGBPixel*)malloc(sizeof(RGBPixel) * grayscale_image->width * grayscale_image->height);

    for (int y = 0; y < grayscale_image->height; y++) {
        for (int x = 0; x < grayscale_image->width; x++) {
            RGBPixel pixel = grayscale_image->data[y * grayscale_image->width + x];
            // Binarizar el pixel basado en el umbral
            if (pixel.r > threshold * 255) {
                pixel.r = pixel.g = pixel.b = 255; // Blanco
            } else {
                pixel.r = pixel.g = pixel.b = 0; // Negro
            }
            binarized_image->data[y * grayscale_image->width + x] = pixel;
        }
    }
    

    return binarized_image;
}


int is_nearly_black(BMPImage* image, float threshold){
    int total_pixels = image->width * image->height;
    int black_pixels = 0;

    // Contar el número de píxeles negros en la imagen
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];
            if (pixel.r <= 10 && pixel.g <= 10 && pixel.b <= 10){
                black_pixels++;
            }
        }
    }

    // Calcular el porcentaje de píxeles negros en la imagen
    float percentage_black = (float)black_pixels / total_pixels;

    // Comprobar si el porcentaje de píxeles negros supera el umbral
    if (percentage_black >= threshold) {
        return 1; // La imagen es casi negra
    } else {
        return 0; // La imagen no es casi negra
    }
}

// Entradas: Un arreglo de punteros a cadenas que contienen los nombres de las imágenes (char** image_names), 
// un arreglo de enteros que contiene las clasificaciones de las imágenes (int* classifications), 
// y el número total de imágenes (int num_images).
// Salidas: Ninguna (void).
// Descripción: Esta función crea un archivo CSV con los resultados de clasificación de las imágenes.
// Toma como entrada un arreglo dinámico de nombres de imágenes y otro de clasificaciones,
// junto con el número total de imágenes. Luego, crea un archivo CSV con dos columnas: una para los nombres de las imágenes
// y otra para sus respectivas clasificaciones. Cada fila del archivo CSV representa una imagen y su clasificación correspondiente.
// El archivo CSV se crea con el nombre especificado en el primer parámetro. Los datos se escriben en el archivo en formato CSV.
// Después de crear el archivo CSV, la función lo cierra correctamente.

// Deja el "filename" como el parametro "-R" que se pide
// El "image_names" es un arreglo de strings que tienes que crear de acuerdo a las imagenes que se pidieron, de ahi saca los nombres
// El "classifications" es un arreglo donde le calculas el is_nearly_black a las imagenes y en ese arreglo guardas los resultados
// EL "num_images" va directamente relacionado con la cantidad de filtros que pida en el "-f", por lo que lo sacas de ahí
void create_csv(const char* filename, char** image_names, int* classifications, int num_images) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo CSV.\n");
        return;
    }

    // Escribir encabezado del archivo CSV
    fprintf(file, "Nombre de la imagen,Clasificación\n");

    // Escribir datos de las imágenes y sus clasificaciones en el archivo CSV
    for (int i = 0; i < num_images; i++) {
        fprintf(file, "%s,%d\n", image_names[i], classifications[i]);
    }

    fclose(file);
}
