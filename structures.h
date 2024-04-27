#ifndef STRUCTURES_H
#define SRUCTURES_H
#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <getopt.h>
#include <unistd.h>

typedef struct Png {
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep *row_pointers;
} Png;

#endif