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
void printPngInfo(Png *image);
void printHelp();
void readPngFile(char *file_name, struct Png *image);
void writePngFile(char *file_name, struct Png *image);
int *getColor(png_bytep *row_pointers, int x, int y);
int* parseColor(char *color);
Png *copy(Png *image, int x1, int y1, int x2, int y2);
void paste(Png *image, Png *area, int x0, int y0);
void drawSimpleCircle(Png *image,int x0, int y0, int radius, int *color);
void setPixel(Png *image, int *color, int x, int y);
void drawLine(Png *image, int x1, int y1, int x2, int y2, char *thickness, int *color);
void drawRectangle(Png *image, int x1, int y1, int x2, int y2, char *thickness, int *color, char *fill, int *fill_color);
void drawOrnament(Png *image, char *pattern, int *color, char *thickness, int count);
void rotateImage(Png *image, int x1, int y1, int x2, int y2, char *angle);


int main(){
    printCWinfo();
    char* input_file = "image.png";
    char* output_file = "file2.png";
    Png image;
    
    readPngFile(input_file, &image);
    char *color = "255.0.0";
    int *arr = parseColor(color);
    //drawRectangle(&image, 300, 200, 400, 100, "1", arr, "a", parseColor("255.0.255"));
    //drawRectangle(&image, 600, 200, 700, 100, "1", arr, NULL, parseColor("0.255.0"));
    rotateImage(&image, 300, 200, 400, 100, "90");
    writePngFile(output_file, &image);
    
    
    return 0;

}

void printCWinfo(){
    printf("Course work for option 4.15, created by Mark Luchkin.\n");
}

void printPngInfo(Png *image){
    printf("Image Width: %d\n", image->width);
    printf("Image Height: %d\n", image->height);

    printf("Color Type: ");
    switch (image->color_type) {
        case PNG_COLOR_TYPE_GRAY:
            printf("Grayscale\n");
            break;
        case PNG_COLOR_TYPE_RGB:
            printf("RGB\n");
            break;
        case PNG_COLOR_TYPE_PALETTE:
            printf("Palette\n");
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            printf("Grayscale with Alpha\n");
            break;
        case PNG_COLOR_TYPE_RGBA:
            printf("RGB with Alpha\n");
            break;
        default:
            printf("Unknown\n");
            break;
    }

    printf("Bit Depth: %d\n", image->bit_depth);
    printf("Number of passes: %d\n", image->number_of_passes);

}

void printHelp(){
    printf("Course work for option 4.15, created by Mark Luchkin.\n");

    printf("Options:\n");
    printf("  -h, --help                Display this help message\n");
    printf("  --info                    Print detailed information about the input PNG file\n");
}

void readPngFile(char *file_name, struct Png *image){
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
        printf("Error: Can't allocate memory for image->row_pointers\n");
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

void writePngFile(char *file_name, struct Png *image){
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

int *getColor(png_bytep *row_pointers, int x, int y){
    int *arr = malloc(sizeof(int) * 3);

    arr[0] = row_pointers[y][x * 3 + 0];
    arr[1] = row_pointers[y][x * 3 + 1];
    arr[2] = row_pointers[y][x * 3 + 2];

    return arr;
}

int *parseColor(char *color) {
    if (color[strlen(color) - 1] == '.' || color[0] == '.' || strstr(color, ".") == NULL){
        printf("Error: not valid color.\n");
        exit(0);
    }

    int *arr = malloc(sizeof(int) * 3);
    sscanf(color, "%d.%d.%d", &arr[0], &arr[1], &arr[2]);

    for (int i = 0; i < 3; i++){
        if (arr[i] > 255 || 0 > arr[i]){
            printf("Error: not valid color.\n");
            exit(0);
        }
    }

    return arr;
}

Png *copy(Png *image, int x1, int y1, int x2, int y2){
    Png *copy_area = malloc(sizeof(Png));
    if (!copy_area){
        printf("Error: Can't allocate memory for area\n");
        exit(0);
    }
    
    copy_area->height = abs(y1 - y2);
    copy_area->width = abs(x2 - x1);
    copy_area->row_pointers = malloc(sizeof(png_bytep) * copy_area->height);
    
    if (!copy_area->row_pointers){
        printf("Error: Can't allocate memory for area's row_pointers\n");
        exit(0);
    }
    
    for (int y = 0; y < copy_area->height; y++) {
        copy_area->row_pointers[y] = malloc(sizeof(png_byte) * copy_area->width * 3);
        if (!copy_area->row_pointers[y]) {
            printf("Error: Can't allocate memory for area pixel\n");
            exit(0);
        }

    }
    
    for (int y = 0; y < copy_area->height; y++){
        for (int x = 0; x < copy_area->width; x++){
            copy_area->row_pointers[y][x * 3 + 0] = image->row_pointers[y + y2][(x + x1) * 3 + 0];
            copy_area->row_pointers[y][x * 3 + 1] = image->row_pointers[y + y2][(x + x1) * 3 + 1];
            copy_area->row_pointers[y][x * 3 + 2] = image->row_pointers[y + y2][(x + x1) * 3 + 2];
        }
    }

    return copy_area;
}

void paste(Png *image, Png *area, int x0, int y0){
    for (int y = 0; y < area->height; y++) {
        for (int x = 0; x < area->width; x++) {            
            image->row_pointers[y + y0][(x + x0) * 3 + 0] = area->row_pointers[y][x * 3 + 0];
            image->row_pointers[y + y0][(x + x0) * 3 + 1] = area->row_pointers[y][x * 3 + 1];
            image->row_pointers[y + y0][(x + x0) * 3 + 2] = area->row_pointers[y][x * 3 + 2];
        }
    }
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
    if (y1 < y2){
        int t = y2;
        y2 = y1;
        y1 = t;
    }

    if (x2 < x1){
        int tp = x1;
        x1 = x2;
        x2 = tp;
    }

    drawLine(image, x1, y1, x2, y1, thickness, color);
    drawLine(image, x1, y2, x2, y2, thickness, color);
    drawLine(image, x2, y2, x2, y1, thickness, color);
    drawLine(image, x1, y2, x1, y1, thickness, color);

    if (fill){
        for (int x = x1 + 1; x < x2; x++){
            for (int y = y1 - 1; y > y2; y--){
                setPixel(image, fill_color, x, y);
            }
        }
    }
}

void drawOrnament(Png *image, char *pattern, int *color, char *thickness, int count){}

void rotateImage(Png *image, int x1, int y1, int x2, int y2, char *angle){
    if (y1 < y2){
        int t = y2;
        y2 = y1;
        y1 = t;
    }

    if (x2 < x1){
        int tp = x1;
        x1 = x2;
        x2 = tp;
    }

    int i_angle = atoi(angle);

    if (i_angle != 90 && i_angle != 180 && i_angle != 270) {
        printf("Error: Unexpected angle.\n");
        exit(44);
    }
    if (x2 == x1 || y2 == y1) {
        printf("Error: Bad rotation area.\n");
        exit(44);
    }

       
    Png *area_to_rotate = copy(image, x1, y1, x2, y2);   
    

    paste(image, area_to_rotate, x1 + 200, y2);

}
