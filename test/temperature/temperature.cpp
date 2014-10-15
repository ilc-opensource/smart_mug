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
 
  char temp[5];

  sprintf(temp, "%d", t);

  canvas.draw_text(0, 0, temp, cyan, black);

  mug_disp_cimg(disp_handle, (cimg_handle_t)&canvas);
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

  mug_temp_on(temp_handle, on_temp, 1000);
  mug_run_temp_watcher(temp_handle);

  return 0;
}
