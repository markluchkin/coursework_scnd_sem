#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <png.h>
#include <getopt.h>

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

    int help;
    int flag_info;
    int flag_input;
    int flag_output;
    int flag_rect;
    int flag_ornament;
    int flag_rotate;
    int flag_left_up; 
    int flag_right_down; 
    int flag_color; 
    int flag_thickness; 
    int flag_fill;
    int flag_fill_color;
    int flag_pattern;
    int flag_count;
    int flag_angle;

    char *left_up_value;
    char *right_down_value;
    char *color_value;
    char *thickness_value;
    char *fill_value;
    char *fill_color_value;
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
void processArguments(int argc, char *argv[], Options *options);
void process(Options *options, Png *image);
int *getColor(png_bytep *row_pointers, int x, int y);
int *parseColor(char *color);
int *parseCoordinates(char *left_up, char *right_down);
Png *copy(Png *image, int x1, int y1, int x2, int y2);
void paste(Png *image, Png *area, int x0, int y0);
void drawSimpleCircle(Png *image,int x0, int y0, int radius, int *color);
int checkCoordinates(Png *image, int x, int y);
void checkThickness(char *thickness);
int checkInCircle(int x, int y, int x0, int y0, int radius, int thickness);
int checkOnCircleLine(int x, int y, int x0, int y0, int radius, int thickness);
void setPixel(Png *image, int *color, int x, int y);
void drawCircle(Png *image, int x1, int y1, int radius, char *thickness, int *color, char *fill, int *fill_color);
void drawUCircle(Png *image, int x1, int y1, int radius, int *color);
void drawLine(Png *image, int x1, int y1, int x2, int y2, char *thickness, int *color);
void drawRectangle(Png *image, int x1, int y1, int x2, int y2, char *thickness, int *color, char *fill, int *fill_color);
void drawOrnament(Png *image, char *pattern, int *color, char *thickness, int count);
void rotateImage(Png *image, int x1, int y1, int x2, int y2, char *angle);


int main(int argc, char *argv[]){
    Png image;
    
    Options options = {NULL};
    options.output_file = "out.png";
    processArguments(argc, argv, &options);
    
    readPngFile(options.input_file, &image);
    
    process(&options, &image);
    
    writePngFile(options.output_file, &image);
    
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

void printHelp() {
    printCWinfo();
    printf("Options:\n");
    printf("  -h, --help                Display this help message\n");
    printf("  --info                    Print detailed information about the input PNG file\n");
    printf("  --input                   Input file name");
    printf("  --output                  Output file name");
    printf("Functions for processing images:\n");

    printf("  Drawing a rectangle:\n");
    printf("    --rect                  Draw a rectangle\n");
    printf("    --left_up               Coordinate of the top-left corner (format: x.y)\n");
    printf("    --right_down            Coordinate of the bottom-right corner (format: x.y)\n");
    printf("    --thickness             Line thickness\n");
    printf("    --color                 Line color (format: rrr.ggg.bbb)\n");
    printf("    --fill                  Optional flag for filling the rectangle (true/false)\n");
    printf("    --fill_color            Fill color (format: rrr.ggg.bbb)\n\n");

    printf("  Creating an ornament frame:\n");
    printf("    --ornament              Create an ornament frame\n");
    printf("    --pattern               Pattern type (rectangle, circle, semicircles, or custom)\n");
    printf("    --color                 Line color (format: rrr.ggg.bbb)\n");
    printf("    --thickness             Line thickness\n");
    printf("    --count                 Number of patterns\n\n");

    printf("  Rotating part of the image:\n");
    printf("    --rotate                Rotate part of the image\n");
    printf("    --left_up               Coordinate of the top-left corner of the area (format: x.y)\n");
    printf("    --right_down            Coordinate of the bottom-right corner of the area (format: x.y)\n");
    printf("    --angle                 Angle of rotation (90, 180, 270)\n");
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
        printf("Error: Can't read file: %s\n", file_name);
        fclose(fp);
        exit(44);
    }
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)){
        printf("Error: Probably, %s is not a png\n", file_name);
        fclose(fp);
        exit(44);
    }

    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!image->png_ptr){
        printf("Error: Error in png structure\n");
        fclose(fp);
        exit(44);
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        printf("Error: Error in png info-structure\n");
        fclose(fp);
        exit(44);
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: Unknown\n");
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
    FILE *fout = fopen(file_name, "wb");
    if (!fout){
        printf("Error: File could not be opened.");
        exit(44);
    }

    png_structp png_ptr_ = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr_){
        printf("Error: png_create_write_struct failed.");
        exit(44);
    }

    png_infop info_ptr_ = png_create_info_struct(png_ptr_);
    if (!info_ptr_) {
        png_destroy_info_struct(png_ptr_, (png_infopp)NULL);
        fclose(fout);
        printf("Error: (info_ptr) png_create_info_struct failed.");
        exit(44);
    }

    if (setjmp(png_jmpbuf(png_ptr_))) {
        png_destroy_info_struct(png_ptr_, &info_ptr_);
        fclose(fout);
        printf("Error: png_init_io failed.");
        exit(44);
    }

    png_init_io(png_ptr_, fout);

    if (setjmp(png_jmpbuf(png_ptr_))) {
        png_destroy_info_struct(png_ptr_, &info_ptr_);
        fclose(fout);
        printf("Error: png_set_IHDR failed.");
        exit(44);
    }

    png_set_IHDR(png_ptr_, info_ptr_, image->width, image->height, image->bit_depth, image->color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr_, info_ptr_);

    if (setjmp(png_jmpbuf(png_ptr_))) {
        png_destroy_info_struct(png_ptr_, &info_ptr_);
        fclose(fout);
        printf("Error: png_write_image failed.");
        exit(44);
    }
    png_write_image(png_ptr_, image->row_pointers);

    if (setjmp(png_jmpbuf(png_ptr_))) {
        png_destroy_info_struct(png_ptr_, &info_ptr_);
        fclose(fout);
        printf("Error: png_write_end failed.");
        exit(44);
    }

    png_write_end(png_ptr_, NULL);
    
    png_destroy_write_struct(&png_ptr_, &info_ptr_);

    fclose(fout);
}

void processArguments(int argc, char *argv[], Options *options){
    opterr = 0;
    const char *short_options = "hi:o:";
    const struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"input", required_argument, NULL, 'i'},
        {"output", required_argument, NULL, 'o'},
        {"rect", no_argument, NULL, 256},
        {"left_up", required_argument, NULL, 257},
        {"right_down", required_argument, NULL, 258},
        {"thickness", required_argument, NULL, 259},
        {"color", required_argument, NULL, 260},
        {"fill", no_argument, NULL, 261},
        {"fill_color", required_argument, NULL, 262},
        {"ornament", no_argument, NULL, 263},
        {"pattern", required_argument, NULL, 264},
        {"count", required_argument, NULL, 265},
        {"rotate", no_argument, NULL, 266},
        {"angle", required_argument, NULL, 267},
        {"info", no_argument, NULL, 268},
        {NULL, 0, NULL, 0}
    };
    int result = 0;
    
    while((result = getopt_long(argc, argv, short_options, long_options, NULL)) != -1){
        switch (result)
        {
        case 'h':
            printHelp();
            exit(EXIT_SUCCESS);
            break;
        
        case 'i':
            options->input_file = optarg;
            options->flag_input = 1;
            break;

        case 'o':
            options->output_file = optarg;
            options->flag_output = 1;
            break;

        case 256:
            if (options->flag_ornament || options->flag_rotate || options->flag_info){
                printf("Error: Can't use more than one function at the same time.\n");
                exit(44);
            }
            options->flag_rect = 1;
            break;

        case 257:
            options->left_up_value = optarg;
            options->flag_left_up = 1;
            break;

        case 258:
            options->right_down_value = optarg;
            options->flag_right_down = 1;
            break;
        
        case 259:
            options->thickness_value = optarg;
            options->flag_thickness = 1;
            break;

        case 260:
            options->color_value = optarg;
            options->flag_color = 1;
            break;

        case 261:
            options->flag_fill = 1;
            break;

        case 262:
            options->fill_color_value = optarg;
            options->flag_fill_color = 1;
            break;

        case 263:
            if (options->flag_rect || options->flag_rotate || options->flag_info){
                printf("Error: Can't use more than one function at the same time.\n");
                exit(44);
            }
            options->flag_ornament = 1;
            break;

        case 264:
            options->pattern_value = optarg;
            options->flag_pattern = 1;
            break;

        case 265:
            options->count_value = optarg;
            options->flag_count = 1;
            break;

        case 266:
            if (options->flag_ornament || options->flag_rect || options->flag_info){
                printf("Error: Can't use more than one function at the same time.\n");
                exit(44);
            }
            options->flag_rotate = 1;
            break;

        case 267:
            options->angle_value = optarg;
            options->flag_angle = 1;
            break;

        case 268:
            if (options->flag_ornament || options->flag_rect || options->flag_rotate){
                printf("Error: Can't use more than one function at the same time.\n");
                exit(44);
            }
            options->flag_info = 1;


        default:
            printf("Error: Unknown option or missing argument\n");
            exit(44);
            break;
        }
    }
    
    if (!options->input_file){
        printf("Error: No input file argument.\n");
        exit(44);
    }

    if (strcmp(options->input_file, options->output_file) == 0) {
        printf("Error: Input and output files can't have the same name.\n");
        exit(44);
    }

    if (!options->flag_info && !options->help && !options->flag_rect && !options->flag_ornament && !options->flag_rotate) {
        printf("Error: No function selected.\n");
        exit(44);
    }

    if (options->flag_rect && (!options->flag_left_up || !options->flag_right_down || !options->flag_left_up || !options->flag_thickness || !options->flag_color)){
        printf("Error: Missing required argument.\n");
        exit(44);
    }

    if (options->flag_fill && !options->flag_fill_color){
        printf("Error: Missing required argument.\n");
        exit(44);
    }

    if (options->flag_ornament && (!options->flag_pattern || !options->flag_color)){
        printf("Error: Missing required argument.\n");
        exit(44);
    }
    
    if (options->flag_ornament && ((strcmp("rectangle", options->pattern_value) == 0 || strcmp("semicircles", options->pattern_value) == 0) && (!options->flag_count || !options->flag_thickness))){
        printf("Error: Missing required argument.\n");
        exit(44);
    }

    if (options->flag_ornament && strcmp("circle", options->pattern_value) == 0){
        options->thickness_value = "1";
        options->count_value = "1";
    }

    if (options->flag_rotate && (!options->flag_angle || !options->flag_left_up || !options->flag_right_down)){
        printf("Error: Missing required argument.\n");
        exit(44);
    }

    

}

void process(Options *options, Png *image){
    if (options->flag_info){
        printPngInfo(image);
        //exit(EXIT_SUCCESS);
    }

    if (options->flag_rect){
        int *coords = parseCoordinates(options->left_up_value, options->right_down_value);
        if ((!options->flag_fill && !options->flag_fill_color) || (options->fill_color_value && !options->flag_fill)){
            drawRectangle(image, coords[0], coords[1], coords[2], coords[3], options->thickness_value, parseColor(options->color_value), NULL, NULL);
            //exit(EXIT_SUCCESS);
        }
        else{
            drawRectangle(image, coords[0], coords[1], coords[2], coords[3], options->thickness_value, parseColor(options->color_value), "fill", parseColor(options->fill_color_value));
            //exit(EXIT_SUCCESS);
        }
    }

    if (options->flag_ornament){
        drawOrnament(image, options->pattern_value, parseColor(options->color_value), options->thickness_value, atoi(options->count_value));
        //exit(EXIT_SUCCESS);
    }

    if (options->flag_rotate){
        int *coords = parseCoordinates(options->left_up_value, options->right_down_value);
        rotateImage(image, coords[0], coords[1], coords[2], coords[3], options->angle_value);
        //exit(EXIT_SUCCESS);
    }
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
        printf("Error: Not valid color.\n");
        exit(44);
    }

    int *arr = malloc(sizeof(int) * 3);
    sscanf(color, "%d.%d.%d", &arr[0], &arr[1], &arr[2]);

    for (int i = 0; i < 3; i++){
        if (arr[i] > 255 || 0 > arr[i]){
            printf("Error: Not valid color.\n");
            exit(44);
        }
    }

    return arr;
}

int *parseCoordinates(char *left_up, char *right_down){
    int *arr = malloc(sizeof(int) * 4);
    if (!arr) {
        printf("Error: Can't allocate memory for coordinates.\n");
        exit(44);
    }
    sscanf(left_up, "%d.%d", &arr[0], &arr[1]);
    sscanf(right_down, "%d.%d", &arr[2], &arr[3]);
    /* x1 y1 x2 y2 format */ 
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

            int *color = getColor(image->row_pointers, x + x1, y + y2);
            setPixel(copy_area, color, x, y);

            // copy_area->row_pointers[y][x * 3 + 0] = image->row_pointers[y + y2][(x + x1) * 3 + 0];
            // copy_area->row_pointers[y][x * 3 + 1] = image->row_pointers[y + y2][(x + x1) * 3 + 1];
            // copy_area->row_pointers[y][x * 3 + 2] = image->row_pointers[y + y2][(x + x1) * 3 + 2];
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
            checkCoordinates(image, x + x0, y + y0);
            int *color = getColor(area->row_pointers, x , y );
            setPixel(image, color, x + x0, y + y0);

            // image->row_pointers[y + y0][(x + x0) * 3 + 0] = area->row_pointers[y][x * 3 + 0];
            // image->row_pointers[y + y0][(x + x0) * 3 + 1] = area->row_pointers[y][x * 3 + 1];
            // image->row_pointers[y + y0][(x + x0) * 3 + 2] = area->row_pointers[y][x * 3 + 2];
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
    int circle_thickness = atoi(thickness) + 1;
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
        for (int x = x1 + atoi(thickness)/2; x < x2 - atoi(thickness)/2; x++){
            for (int y = y1 - atoi(thickness)/2; y > y2 + atoi(thickness)/2; y--){
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
        for (int i = 1; i <= count; i++){
            x0 = (i - 1) * 2 * i_thickness;
            y0 = x0;
            x1 = w - x0 - 1;
            y1 = h - y0 - 1;

            if (x0 <= x1 && y0 <= y1){
                drawRectangle(image, x0, y0, x1, y1, "1", color, NULL, NULL);
                drawRectangle(image, x0 + i_thickness - 1, y0 + i_thickness - 1, x1 - i_thickness + 1, y1 - i_thickness + 1, "1",color, NULL, NULL );
                for (int j = 0; j < i_thickness; j++){
                    drawRectangle(image, x0 + j, y0 + j, x1 - j, y1 - j, "1", color, NULL, NULL);
                }
            }
        }

    } else if (strcmp(pattern, "circle") == 0){
        int radius = MIN(w, h) / 2 - 1;
        drawUCircle(image, w / 2, h / 2, radius, color);
        
    } else if (strcmp(pattern, "semicircles") == 0){

        // double width = (double)(w - count * i_thickness) / (count * 2);
        // double height = (double)(h - count * i_thickness) / (count * 2);
        int w_radius;
        int h_radius;
        if ((h / count) % 2 == 0){
           h_radius = (h / count / 2);
        }
        else{
            h_radius = (h / count / 2) + 1;
        }

        if ((w / count) % 2 == 0){
           w_radius = (w / count / 2);
        }
        else{
            w_radius = (w / count / 2) + 1;
        }

        printf("w %d h %d\n", w, h);
        int x = w_radius, y = h_radius;
        for (int i = 0; i <= count; i++){
            
            drawCircle(image, x, 0, w_radius, thickness, color, NULL, NULL);
            drawCircle(image, x, h - 1, w_radius, thickness, color, NULL, NULL);
            x += 2 * w_radius;
            
        }
        for (int j = 0; j <= count; j++){
            
            drawCircle(image, 0, y, h_radius, thickness, color, NULL, NULL);
            drawCircle(image, w - 1, y, h_radius, thickness, color, NULL, NULL);
            y += 2 * h_radius ;
            
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
    

    if (!(0 <= x2 && x2 <= image->width && 0 <= y2 && y2 <= image->height)) {
        printf("Error: Rotation area is not on image.\n");
        exit(44);
    }
       
    Png *area_to_rotate = copy(image, MIN(x2, x1), MAX(y2, y1), MAX(x1, x2), MIN(y2, y1)); 

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
