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
handle_t motion_handle;

void clear_canvas()
{
  canvas.draw_rectangle(0, 0, 0,
                        SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                        black);

}

void draw_data(int t)
{
  clear_canvas();
 
  char temp[5];

  sprintf(temp, "%d", t);

  canvas.draw_text(0, 0, temp, cyan, black);

  mug_disp_cimg(disp_handle, (cimg_handle_t)&canvas);
}


void on_motion_angle(float angle_x, float angle_y, float angle_z)
{
  printf("%f, %f, %f\n", angle_x, angle_y, angle_z);
  draw_data((int)angle_z);
}

int main(int argc, char** argv)
{
  disp_handle = mug_disp_init();
  motion_handle = mug_motion_init();

  mug_motion_angle_on(motion_handle, on_motion_angle);
 
  mug_run_motion_watcher(motion_handle);
  return 0;
}
