#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <getopt.h>
#include <unistd.h>
#define PI 3.14159265

struct Png{
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep *row_pointers;
};


void printCWinfo();
void printHelp();
void read_png_file(char *file_name, struct Png *image);
void write_png_file(char *file_name, struct Png *image);
void drawRectangle();
void rotateImage();

int main(int argc, char** argv){
    printCWinfo();

    return 0;
}

void printCWinfo(){
    printf("Course work for option 4.15, created by Mark Luchkin.\n");
}

void printHelp(){}

void read_png_file(char *file_name, struct Png *image){
    int x,y;
    char header[8]; 
    
    FILE *fp = fopen(file_name, "rb");
    if (!fp) {
        printf("Error: can't read file: %s\n", file_name);
        fclose(fp);
        exit(0);
    }
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)){
        printf("Error: probably, %s is not a png\n", file_name);
        fclose(fp);
        exit(0);
    }

    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!image->png_ptr){
        printf("Error: error in png structure\n");
        fclose(fp);
        exit(0);
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        printf("Error: error in png info-structure\n");
        fclose(fp);
        exit(0);
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: unknown\n");
        fclose(fp);
        exit(0);
    }

    png_init_io(image->png_ptr, fp);
    png_set_sig_bytes(image->png_ptr, 8);
    png_read_info(image->png_ptr, image->info_ptr);
    image->width = png_get_image_width(image->png_ptr, image->info_ptr);
    image->height = png_get_image_height(image->png_ptr, image->info_ptr);
    image->color_type = png_get_color_type(image->png_ptr, image->info_ptr);
    image->bit_depth = png_get_bit_depth(image->png_ptr, image->info_ptr);
    image->number_of_passes = png_set_interlace_handling(image->png_ptr);
    png_read_update_info(image->png_ptr, image->info_ptr);

    image->row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * image->height);
    if (!image->row_pointers) {
        printf("Error: Can not allocate memory for image->row_pointers\n");
        exit(0);
    }

    for (y = 0; y < image->height; y++){
        image->row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(image->png_ptr, image->info_ptr));
        if (!image->row_pointers[y]){
            printf("Error: can't allocate memory for pixel");
            exit(0);
        }
    }   
    png_read_image(image->png_ptr, image->row_pointers);
    fclose(fp);
    
}

void write_png_file(char *file_name, struct Png *image){
    int x, y;
    FILE* fp = fopen(file_name, "wb");
    if (!fp){
                printf("Error: can't create file: %s\n", file_name);
        fclose(fp);
        exit(0);
    }

    image->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!image->png_ptr){
        printf("Error: error in png structure\n");
        fclose(fp);
        exit(0);
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        printf("Error: error in png info-structure\n");
        fclose(fp);
        exit(0);
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: unknown\n");
        fclose(fp);
        exit(0);
    }

    png_init_io(image->png_ptr, fp);
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: unknown\n");
        fclose(fp);
        exit(0);
    }
    png_set_IHDR(image->png_ptr, image->info_ptr, image->width, image->height, image->bit_depth, image->color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(image->png_ptr, image->info_ptr);
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: unknown\n");
        fclose(fp);
        exit(0);
    }

    png_write_image(image->png_ptr, image->row_pointers);
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: unknown\n");
        fclose(fp);
        exit(0);
    }

    png_write_end(image->png_ptr, NULL);

    for (y = 0; y < image->height; y++)
        free(image->row_pointers[y]);
    free(image->row_pointers);
    fclose(fp);

}

void drawRectangle(){}

void rotateImage(){}