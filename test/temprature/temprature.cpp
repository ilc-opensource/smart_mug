#define cimg_display 0
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mug.h>

#include <CImg.h>
using namespace cimg_library;

CImg<unsigned char> canvas(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 3, 0);

#define WARM 30
#define HOT  50

handle_t disp_handle;
handle_t temp_handle;

void clear_canvas()
{
  canvas.draw_rectangle(0, 0, 0,
                        SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                        black);

}

void draw_temp(int t)
{
  clear_canvas();
  int width, height;
  mug_number_text_shape(&width, &height);

  char buf[4];

  sprintf(buf, "%d", t);

  unsigned char * color;

  if( t < WARM)
    color = cyan;
  else if(t < HOT)
    color = yellow;
  else
    color = red; 

  mug_draw_number_cimg(&canvas, 0, 0, buf, color);
  mug_disp_cimg(disp_handle, &canvas);
}

void on_temp(int board_temp, int mug_temp, int battery_temp)
{
  printf("%d, %d, %d\n", board_temp, mug_temp, battery_temp);
  draw_temp(board_temp);
}

int main(int argc, char** argv)
{
  temp_data_t data;

  disp_handle = mug_disp_init();
  temp_handle = mug_temp_init();

#ifdef USE_LIBUV
  mug_temp_on(temp_handle, on_temp, 1000);
  mug_run_temp_watcher(temp_handle);
#else
  printf("handle: 0%x\n", temp_handle);
 
  while(1) {
#if 0
    mug_read_temp(handle, &data);
    printf("board: %4d, mug: %4d, battery: %4d\n", data.board_temp, data.mug_temp, data.battery_temp);
#else
   int temp = mug_read_board_temp(temp_handle);
   printf("board temp: %d\n", temp);
#endif
    usleep(500*1000);
  }
#endif
  return 0;
}
