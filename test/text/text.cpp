#define cimg_display 0 
#include <stdlib.h>
#include <pthread.h>
#include "CImg.h"
using namespace cimg_library;

#if !cimg_display
#include <mug.h>
#endif

#include <vector>
using namespace std;

unsigned char red[]    = {255, 0, 0};
unsigned char green[]  = {0, 255, 0};
unsigned char blue[]   = {0, 0, 255};
unsigned char bg[]     = {0, 0, 0};
unsigned char yellow[] = {255, 255, 0};
unsigned char* colors[] = {red, green, blue, yellow};

typedef unsigned char * color_t;
#define COLOR_IDLE  blue
#define COLOR_GOOD  green
#define COLOR_WRONG red
#define COLOR_BG    bg

CImg<unsigned char> canvas(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 3, 0);

int main(int argc, char** argv)
{
  if(argc < 3) {
    printf("no input text\n");
    return 1;
  }

  int size = atoi(argv[1]);  
  char *content = argv[2];

  char temp[4];

  int width, height;
  mug_number_text_shape(&width, &height);

  for(int i = 0; i < 4; i++) {
    sprintf(temp, "%d", i);
    mug_draw_number_cimg(&canvas, i * width, 0, temp, colors[i]) ;
  }

  handle_t handle = mug_disp_init();

  mug_disp_cimg(handle,  &canvas);

  return 0;
}
