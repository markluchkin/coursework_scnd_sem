#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <png.h>
#include <getopt.h>
#include <unistd.h>
#define PI 3.14159265

typedef struct Png{
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep *row_pointers;
} Png;


void printCWinfo();
void printHelp();
void read_png_file(char *file_name, struct Png *image);
void write_png_file(char *file_name, struct Png *image);
int* getColor(char *color);
void setPixel(Png *image, int *color, int x, int y);
void drawLine(Png *image, int x1, int y1, int x2, int y2, char *thickness, int *color);
void drawRectangle(Png *image, int x1, int y1, int x2, int y2, char *thickness, int *color, char *fill, int *fill_color);
void drawOrnament(Png *image, char *pattern, int *color, char *thickness, int count);
void rotateImage(Png *image, int x1, int y1, int x2, int y2, char *angle);


int main(){
    printCWinfo();
    char* input_file = "file.png";
    char* output_file = "file2.png";
    Png image;
    read_png_file(input_file, &image);
    char *color = "255.0.0";
    int *arr = getColor(color);
    drawRectangle(&image, 550, 200, 650, 100, "50", arr, "false", NULL);
    write_png_file(output_file, &image);

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
    png_destroy_write_struct(&(image->png_ptr), &(image->info_ptr));

}

int *getColor(char *color) {
    int *arr = malloc(sizeof(int) * 3);
    sscanf(color, "%d.%d.%d", &arr[0], &arr[1], &arr[2]);
    return arr;
}


void drawSimpleCircle(Png *image,int x0, int y0, int radius, int *color){
    int D = 3 - 2 * radius;
    int x = 0;
    int y = radius;
    while (x <= y) {
        setPixel(image, color, x+x0, y+y0);
        setPixel(image, color, y+x0, x+y0);
        setPixel(image, color, -y+x0, x+y0);
        setPixel(image, color, -x+x0, y+y0);
        setPixel(image, color, -x+x0, -y+y0);
        setPixel(image, color, -y+x0, -x+y0);
        setPixel(image, color, y+x0, -x+y0);
        setPixel(image, color, x+x0, -y+y0);

        if (D < 0) {
            D += 4 * x + 6;
            x++;
        } else {
            D += 4 * (x - y) + 10;
            x++;
            y--;
        }
    }
}
void checkThickness(char *thickness){
    int line_thickness = atoi(thickness);
    if (line_thickness <= 0){
        printf("Error: thikness must be a positive integer\n");
        exit(0);
    }
    return;
}

void setPixel(Png *image, int *color, int x, int y){
    image->row_pointers[y][x * 3 + 0] = color[0];
    image->row_pointers[y][x * 3 + 1] = color[1];
    image->row_pointers[y][x * 3 + 2] = color[2];
}

void drawLine(Png *image, int x1, int y1, int x2, int y2, char *thickness, int *color){
    checkThickness(thickness);
    int line_thickness = atoi(thickness);
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    int h = image->height;
    int w = image->width;
    while(1){
        if (y1 >= 0 && y1 <= h && x1 >= 0 && x1 <= w){
            if (line_thickness == 1){
                setPixel(image, color, x1, y1);
            }   
        }

        if(line_thickness > 1 && x1 - (line_thickness/2) < w && y1 - (line_thickness/2) < h && x1 + (line_thickness/2) >= 0 && y1 + (line_thickness/2) >= 0){
            drawSimpleCircle(image, x1, y1, line_thickness/2 ,color);
        }

        if (x1 == x2 && y1 == y2){
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}


void drawRectangle(Png *image, int x1, int y1, int x2, int y2, char *thickness, int *color, char *fill, int *fill_color){
    drawLine(image, x1, y1, x2, y1, thickness, color);
    drawLine(image, x1, y2, x2, y2, thickness, color);
    drawLine(image, x2, y2, x2, y1, thickness, color);
    drawLine(image, x1, y2, x1, y1, thickness, color);
}

void drawOrnament(Png *image, char *pattern, int *color, char *thickness, int count){}

void rotateImage(Png *image, int x1, int y1, int x2, int y2, char *angle){}
