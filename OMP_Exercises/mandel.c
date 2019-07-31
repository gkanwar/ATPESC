/*
**  PROGRAM: Mandelbrot area
**
**  PURPOSE: Program to compute the area of a  Mandelbrot set.
**           Correct answer should be around 1.510659.
**           WARNING: this program may contain errors
**
**  USAGE:   Program runs without input ... just run the executable
**            
**  HISTORY: Written:  (Mark Bull, August 2011).
**           Changed "comples" to "d_comples" to avoid collsion with 
**           math.h complex type (Tim Mattson, September 2011)
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <omp.h>

# define NPOINTS 5000
# define MAXITER 1000

struct d_complex{
  double r;
  double i;
};

int testpoint(struct d_complex);

struct rgb_data {
  float r, g, b;
};

void save_bitmap(const char *file_name, int width, int height, int dpi, struct rgb_data *pixel_data) {
  // create a file object that we will use to write our image
  FILE *image;
  // we want to know how many pixels to reserve
  int image_size = width * height;
  // a byte is 4 bits but we are creating a 24 bit image so we can represent a pixel with 3
  // our final file size of our image is the width * height * 4 + size of bitmap header
  int file_size = 54 + 4 * image_size;
  // pixels per meter https://www.wikiwand.com/en/Dots_per_inch
  int ppm = dpi * 39.375;

  // bitmap file header (14 bytes)
  // we could be savages and just create 2 array but since this is for learning lets
  // use structs so it can be parsed by someone without having to refer to the spec

  // since we have a non-natural set of bytes, we must explicitly tell the
  // compiler to not pad anything, on gcc the attribute alone doesn't work so
  // a nifty trick is if we declare the smallest data type last the compiler
  // *might* ignore padding, in some cases we can use a pragma or gcc's
  // __attribute__((__packed__)) when declaring the struct
  // we do this so we can have an accurate sizeof() which should be 14, however
  // this won't work here since we need to order the bytes as they are written
  struct bitmap_file_header {
    unsigned char   bitmap_type[2];     // 2 bytes
    int             file_size;          // 4 bytes
    short           reserved1;          // 2 bytes
    short           reserved2;          // 2 bytes
    unsigned int    offset_bits;        // 4 bytes
  } bfh;

  // bitmap image header (40 bytes)
  struct bitmap_image_header {
    unsigned int    size_header;        // 4 bytes
    unsigned int    width;              // 4 bytes
    unsigned int    height;             // 4 bytes
    short int       planes;             // 2 bytes
    short int       bit_count;          // 2 bytes
    unsigned int    compression;        // 4 bytes
    unsigned int    image_size;         // 4 bytes
    unsigned int    ppm_x;              // 4 bytes
    unsigned int    ppm_y;              // 4 bytes
    unsigned int    clr_used;           // 4 bytes
    unsigned int    clr_important;      // 4 bytes
  } bih;

  // if you are on Windows you can include <windows.h>
  // and make use of the BITMAPFILEHEADER and BITMAPINFOHEADER structs

  memcpy(&bfh.bitmap_type, "BM", 2);
  bfh.file_size       = file_size;
  bfh.reserved1       = 0;
  bfh.reserved2       = 0;
  bfh.offset_bits     = 0;

  bih.size_header     = sizeof(bih);
  bih.width           = width;
  bih.height          = height;
  bih.planes          = 1;
  bih.bit_count       = 24;
  bih.compression     = 0;
  bih.image_size      = file_size;
  bih.ppm_x           = ppm;
  bih.ppm_y           = ppm;
  bih.clr_used        = 0;
  bih.clr_important   = 0;

  image = fopen(file_name, "wb");

  // compiler woes so we will just use the constant 14 we know we have
  fwrite(&bfh, 1, 14, image);
  fwrite(&bih, 1, sizeof(bih), image);

  // write out pixel data, one last important this to know is the ordering is backwards
  // we have to go BGR as opposed to RGB
  for (int i = 0; i < image_size; i++) {
    struct rgb_data BGR = pixel_data[i];

    float red   = (BGR.r);
    float green = (BGR.g);
    float blue  = (BGR.b);

    // if you don't follow BGR image will be flipped!
    unsigned char color[3] = {
      blue, green, red
    };

    fwrite(color, 1, sizeof(color), image);
  }

  fclose(image);
}


int main(){
  int i, j;
  double area, error, eps = 1.0e-5;
  int numoutside = 0;

  //   Loop over grid of points in the complex plane which contains the Mandelbrot set,
  //   testing each point to see whether it is inside or outside the set.

  double start_time = omp_get_wtime();
  struct rgb_data *pixels = (struct rgb_data *) malloc(NPOINTS*NPOINTS*sizeof(struct rgb_data));
  memset(&(pixels[0]), 0, NPOINTS*NPOINTS*sizeof(struct rgb_data));

  const double TA1 = tanh(2.0);

  #pragma omp parallel for schedule(dynamic, NPOINTS/100) default(none) shared(eps,pixels) private(j) reduction(+:numoutside)
  for (i=0; i<NPOINTS; i++) {
    for (j=0; j<NPOINTS; j++) {
      struct d_complex c;
      c.r = -2.0+2.5*(double)(i)/(double)(NPOINTS)+eps;
      c.i = 1.125*(double)(j)/(double)(NPOINTS)+eps;
      int iters = testpoint(c);
      if (iters != MAXITER) {
        numoutside += 1;
        pixels[i + j*NPOINTS].r = (int) (255 * tanh(2 * ((double) iters) / MAXITER) / TA1);
        pixels[i + j*NPOINTS].g = (int) (255 * tanh(2 * ((double) iters) / MAXITER) / TA1);
        pixels[i + j*NPOINTS].b = (int) (255 * tanh(2 * ((double) iters) / MAXITER) / TA1);
      }
    }
  }

  // Calculate area of set and error estimate and output the results
   
  area=2.0*2.5*1.125*(double)(NPOINTS*NPOINTS-numoutside)/(double)(NPOINTS*NPOINTS);
  error=area/(double)NPOINTS;

  double run_time = omp_get_wtime() - start_time;

  printf("Area of Mandlebrot set = %12.8f +/- %12.8f\n",area,error);
  printf("Correct answer should be around 1.510659\n");
  printf("Took %lf seconds\n", run_time);

  save_bitmap("output.bmp", NPOINTS, NPOINTS, 96, pixels);
  system("convert output.bmp -scale 100%\\!x50%\\! output2.bmp");
}

int testpoint(struct d_complex c){

  // Does the iteration z=z*z+c, until |z| > 2 when point is known to be outside set
  // If loop count reaches MAXITER, point is considered to be inside the set

  struct d_complex z;
  int iter;
  double temp;

  z=c;
  for (iter=0; iter<MAXITER; iter++){
    temp = (z.r*z.r)-(z.i*z.i)+c.r;
    z.i = z.r*z.i*2+c.i;
    z.r = temp;
    if ((z.r*z.r+z.i*z.i)>4.0) {
      return iter;
    }
  }
  return MAXITER;
}

