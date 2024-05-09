#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <png.h>
#include <getopt.h>
#include <unistd.h>

#define PI 3.14159265
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

typedef struct Png{
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep *row_pointers;
} Png;

typedef struct Options{
    char *input_file;
    char *output_file;

    char* left_up_value;
    char* right_down_value;
    char *color_value;
    char* thickness_value;
    char *fill_value;
    char *fill_color;
    char *pattern_value;
    char *count_value;
    char *angle_value;

} Options;


void printCWinfo();
void printPngInfo(Png *image);
void printHelp();
Png *createPng(int height, int width);
void readPngFile(char *file_name, struct Png *image);
void writePngFile(char *file_name, struct Png *image);
int *getColor(png_bytep *row_pointers, int x, int y);
int* parseColor(char *color);
Png *copy(Png *image, int x1, int y1, int x2, int y2);
void paste(Png *image, Png *area, int x0, int y0);
void drawSimpleCircle(Png *image,int x0, int y0, int radius, int *color);
int checkCoordinates(Png *image, int x, int y);
void checkThickness(char *thickness);
int checkInCircle(int x, int y, int x0, int y0, int radius, int thickness);
void setPixel(Png *image, int *color, int x, int y);
void drawCircle(Png *image, int x1, int y1, int radius, char *thickness, int *color, char *fill, int *fill_color);
void drawUCircle(Png *image, int x1, int y1, int radius, int *color);
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
    //rotateImage(&image, 300, 200, 400, 100, "90");
    drawOrnament(&image, "semicircles", parseColor("255.0.255"), "1", 1);
    //drawCircle(&image, 400, 150, 50, "10", parseColor("255.0.100"), "true", parseColor("0.150.30"));
    //drawSimpleCircle(&image, 400, 150, 50, parseColor("255.0.0"));
    writePngFile(output_file, &image);
    
    
    return 0;

}

void printCWinfo(){
    printf("Course work for option 4.15, created by Mark Luchkin.\n");
}

void printPngInfo(Png *image){
    if (image == NULL) {
        printf("Error: Null pointer to image.\n");
        exit(44);
    }

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

Png *createPng(int height, int width){
    Png *png = malloc(sizeof(Png));
    if (!png){
        printf("Error: Can't allocate memory for area\n");
        exit(44);
    }
    
    png->height = height;
    png->width = width;
    png->row_pointers = malloc(sizeof(png_bytep) * png->height);
    
    if (!png->row_pointers){
        printf("Error: Can't allocate memory for area's row_pointers\n");
        exit(44);
    }
    
    for (int y = 0; y < png->height; y++) {
        png->row_pointers[y] = malloc(sizeof(png_byte) * png->width * 3);
        if (!png->row_pointers[y]) {
            printf("Error: Can't allocate memory for area pixel\n");
            exit(44);
        }

    }

    return png;
}

void readPngFile(char *file_name, struct Png *image){
    int x,y;
    char header[8]; 
    
    FILE *fp = fopen(file_name, "rb");
    if (!fp) {
        printf("Error: can't read file: %s\n", file_name);
        fclose(fp);
        exit(44);
    }
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)){
        printf("Error: probably, %s is not a png\n", file_name);
        fclose(fp);
        exit(44);
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
        exit(44);
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
        exit(44);
    }

    for (y = 0; y < image->height; y++){
        image->row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(image->png_ptr, image->info_ptr));
        if (!image->row_pointers[y]){
            printf("Error: can't allocate memory for pixel");
            exit(44);
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
        exit(44);
    }

    image->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!image->png_ptr){
        printf("Error: error in png structure\n");
        fclose(fp);
        exit(44);
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        printf("Error: error in png info-structure\n");
        fclose(fp);
        exit(44);
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: unknown\n");
        fclose(fp);
        exit(44);
    }

    png_init_io(image->png_ptr, fp);
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: unknown\n");
        fclose(fp);
        exit(44);
    }
    png_set_IHDR(image->png_ptr, image->info_ptr, image->width, image->height, image->bit_depth, image->color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(image->png_ptr, image->info_ptr);
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: unknown\n");
        fclose(fp);
        exit(44);
    }

    png_write_image(image->png_ptr, image->row_pointers);
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: unknown\n");
        fclose(fp);
        exit(44);
    }

    png_write_end(image->png_ptr, NULL);

    for (y = 0; y < image->height; y++)
        free(image->row_pointers[y]);

    free(image->row_pointers);
    fclose(fp);
    png_destroy_write_struct(&(image->png_ptr), &(image->info_ptr));

}

int *getColor(png_bytep *row_pointers, int x, int y){
    int *color = malloc(sizeof(int) * 3);

    color[0] = row_pointers[y][x * 3 + 0];
    color[1] = row_pointers[y][x * 3 + 1];
    color[2] = row_pointers[y][x * 3 + 2];

    return color;
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
    if (image == NULL) {
        printf("Error: Null pointer to image.\n");
        exit(44);
    }
    
    Png *copy_area = createPng(abs(y1 - y2), abs(x2 - x1));
        
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
    if (image == NULL) {
        printf("Error: Null pointer to image.\n");
        exit(44);
    }

    for (int y = 0; y < area->height; y++) {
        for (int x = 0; x < area->width; x++) {            
            image->row_pointers[y + y0][(x + x0) * 3 + 0] = area->row_pointers[y][x * 3 + 0];
            image->row_pointers[y + y0][(x + x0) * 3 + 1] = area->row_pointers[y][x * 3 + 1];
            image->row_pointers[y + y0][(x + x0) * 3 + 2] = area->row_pointers[y][x * 3 + 2];
        }
    }
}

void drawSimpleCircle(Png *image,int x0, int y0, int radius, int *color){
    if (image == NULL || color == NULL) {
        printf("Error: Null pointer to image or color.\n");
        exit(44);
    }
    
    int delta = 3 - 2 * radius;
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

        //D += D < 0 ? 4 * x + 6 : 4 * (x - y--) + 10;
        
        if (delta < 0) {
            delta += 4 * x + 6;
            x++;
        } else {
            delta += 4 * (x - y) + 10;
            x++;
            y--;
        }
    }
}

int checkCoordinates(Png *image, int x, int y){
    return (x >= 0 && x < image->width && y >= 0 && y < image->height);
}

void checkThickness(char *thickness){
    int line_thickness = atoi(thickness);
    if (line_thickness <= 0){
        printf("Error: Thikness must be a positive integer\n");
        exit(44);
    }
    return;
}

int checkInCircle(int x, int y, int x0, int y0, int radius, int thickness){
    return (int)((x - x0) * (x - x0) + (y - y0) * (y - y0) <= (radius - thickness / 2) * (radius - thickness / 2));
}

int checkOnCircleLine(int x, int y, int x0, int y0, int radius, int thickness){
    int flag1 = (x-x0)*(x-x0) + (y-y0)*(y-y0) <= (radius+thickness/2)*(radius+thickness/2);
    int flag2 = (x-x0)*(x-x0) + (y-y0)*(y-y0) >= (MAX(0, radius-thickness/2))*(MAX(0, radius-thickness/2));
    return flag1 && flag2;
}

void setPixel(Png *image, int *color, int x, int y){
    if (image == NULL || color == NULL) {
        printf("Error: Null pointer to image or color.\n");
        exit(44);
    }

    if (!checkCoordinates(image, x, y)){
        return;
    }

    image->row_pointers[y][x * 3 + 0] = color[0];
    image->row_pointers[y][x * 3 + 1] = color[1];
    image->row_pointers[y][x * 3 + 2] = color[2];
}

void drawCircle(Png *image, int x1, int y1, int radius, char *thickness, int *color, char *fill, int *fill_color){
    if (image == NULL || color == NULL){
        printf("Error: Null pointer to image or color.\n");
        exit(44);
    }

    checkThickness(thickness);
    int circle_thickness = atoi(thickness);
    int w = image->width;
    int h = image->height;
    for (int y = MAX(0, y1 - radius - circle_thickness/2); y <= MIN(h - 1, y1 + radius + circle_thickness/2); y++){
        for (int x = MAX(0, x1 - radius - circle_thickness/2); x <= MIN(w - 1, x1 + radius + circle_thickness/2); x++){
            if (fill && checkInCircle(x, y, x1, y1, radius, circle_thickness)){
                setPixel(image, fill_color, x, y);
            } 

            if (checkOnCircleLine(x, y, x1, y1, radius, circle_thickness)){
                setPixel(image, color, x, y);
            }

        }
    }

    drawSimpleCircle(image, x1, y1, radius - circle_thickness/2, color );
    drawSimpleCircle(image, x1, y1, radius + circle_thickness/2, color );
}

void drawUCircle(Png *image, int x1, int y1, int radius, int *color){
    if (image == NULL || color == NULL){
        printf("Error: Null pointer to image or color.\n");
        exit(44);
    }

    int w = image->width;
    int h = image->height;
    for (int y = 0; y <= h; y++){
        for (int x = 0; x <= w; x++){
            if (!checkOnCircleLine(x, y, x1, y1, radius, 1) && !checkInCircle(x, y, x1, y1, radius, 1)){
                setPixel(image, color, x, y);
            }
        }
    }

    drawSimpleCircle(image, x1, y1, radius, color);
}

void drawLine(Png *image, int x1, int y1, int x2, int y2, char *thickness, int *color){
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
    if (image == NULL || color == NULL) {
        printf("Error: Null pointer to image or color.\n");
        exit(44);
    }
    
    checkThickness(thickness);

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

void drawOrnament(Png *image, char *pattern, int *color, char *thickness, int count){
    if (image == NULL || color == NULL) {
        printf("Error: Null pointer to image or color.\n");
        exit(44);
    }

    if (count <= 0) {
        printf("Error: Count is less than 1.\n");
        exit(44);
    }

    checkThickness(thickness);
    int i_thickness = atoi(thickness);
    int w = image->width;
    int h = image->height;

    if (strcmp(pattern, "rectangle") == 0){
        int x0, y0, x1, y1;
        for (int i = 1; i <= count; ++i){
            x0 = (i - 1) * 2 * i_thickness;
            y0 = x0;
            x1 = w - x0 - 1;
            y1 = h - y0 - 1;

            if (x0 <= x1 && y0 <= y1){
                drawRectangle(image, x0, y0, x1, y1, "1", color, NULL, NULL);
                drawRectangle(image, x0 + i_thickness - 1, y0 + i_thickness - 1, x1 - i_thickness + 1, y1 - i_thickness + 1, "1", color, NULL, NULL);

            }
        }

    } else if (strcmp(pattern, "circle") == 0){
        int radius = MIN(w, h) / 2 - 1;
        drawUCircle(image, w / 2, h / 2, radius, color);
        
    } else if (strcmp(pattern, "semicircles")){
        int h_radius = h / (2 * count);
        int w_radius = w / (2 * count);
        int x = w_radius, y = h_radius;
        for (int i = 0; i < count; i++){
            drawCircle(image, x, 0, w_radius, thickness, color, NULL, NULL);
            drawCircle(image, x, h, w_radius, thickness, color, NULL, NULL);
            x += 2 * w_radius;
        }
        for (int j = 0; j < count; j++){
            drawCircle(image, 0, y, h_radius, thickness, color, NULL, NULL);
            drawCircle(image, w, y, h_radius, thickness, color, NULL, NULL);
            y += 2 * h_radius;
        }
    }
}

void rotateImage(Png *image, int x1, int y1, int x2, int y2, char *angle){
    if(!image){
        printf("Error: Null pointer to image.\n");
        exit(40);
    }

    int cr_x0 = MIN(x2, x1);
    int cr_y0 = MIN(y2, y1);
    int cr_x1 = MAX(x2, x1);
    int cr_y1 = MAX(y2, y1);
    int i_angle = atoi(angle);

    if (i_angle != 90 && i_angle != 180 && i_angle != 270) {
        printf("Error: Unexpected angle.\n");
        exit(44);
    }
    if (x2 == x1 || y2 == y1) {
        printf("Error: Bad rotation area.\n");
        exit(44);
    }

    if (!(0 <= x2 && x2 <= image->width && 0 <= y2 && y2 <= image->height)) {
        printf("Error: Rotation area is not on image.\n");
        exit(44);
    }
       
    Png *area_to_rotate = copy(image, x1, y1, x2, y2); 

    int src_width = area_to_rotate->width;
    int src_height = area_to_rotate->height;
    int count = i_angle / 90; 
    
    for (int i = 0; i < count; i++){
        int w, h;
        if (i % 2 == 0){
            w = src_height;
            h = src_width;
        } else{
            w = src_width;
            h = src_height;
        }

        Png *rotated_area = createPng(h, w);
        for (int y = 0; y < area_to_rotate->height; ++y) {
            for (int x = 0; x < area_to_rotate->width; ++x) {
                int *color = getColor(area_to_rotate->row_pointers, x, y);
                setPixel(rotated_area, color, y, area_to_rotate->width - 1 - x);
                free(color);
            }
        }

        area_to_rotate = rotated_area;
    }

    int paste_x = (cr_x1 + cr_x0) / 2 - area_to_rotate->width / 2;
    int paste_y = (cr_y1 + cr_y0) / 2 - area_to_rotate->height / 2;

    paste(image, area_to_rotate, paste_x, paste_y);
}
