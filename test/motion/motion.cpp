#define cimg_display 0 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mug.h>
#include <CImg.h>
using namespace cimg_library;

static handle_t disp_handle;
static handle_t motion_handle;

#define MAX_VAL    6000
#define STEP       (MAX_VAL / (SCREEN_HEIGHT / 2))
#define BAR_Y      (SCREEN_HEIGHT / 2)
#define BAR_WIDTH  (SCREEN_WIDTH / 6)

CImg<unsigned char> canvas(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 3, 0);

void clear_canvas()
{
  canvas.draw_rectangle(0, 0, 0,
                        SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                        black);

}

void draw_bar(int idx, int val, unsigned char * color)
{
  if(val >= MAX_VAL)
    val = MAX_VAL;
    
  if(val <= -MAX_VAL)
    val = -MAX_VAL;

  int bar_height = (val / STEP);

  canvas.draw_rectangle(idx * BAR_WIDTH, BAR_Y, 0,
                        (idx+1) * BAR_WIDTH - 1, BAR_Y - bar_height, 0,	   
                        color);

}

void on_motion(int ax, int ay, int az, int gx, int gy, int gz)
{
  printf("async ax:%8d, ay:%8d, az:%8d, gx:%8d, gy:%8d, gz:%8d\n",
          ax, ay, az, gx, gy, gz);
 
  clear_canvas();

  draw_bar(0, ax, red);
  draw_bar(1, ay, green);  
  draw_bar(2, az, blue);    

  draw_bar(3, gx, yellow);
  draw_bar(4, gy, magenta);	
  draw_bar(5, gz, cyan);	  

  mug_disp_cimg(disp_handle, &canvas);
}

void on_angle(float angle_x, float angle_y, float angle_z)
{
  printf("[ %f, %f, %f ]\n", angle_x, angle_y, angle_z);
}

void on_touch(int x, int y, int id)
{
  printf("(%d, %d, %d)\n", x, y, id);
}

void init()
{
  disp_handle = mug_disp_init();
  motion_handle = mug_motion_init();
}

int main(int argc, char** argv)
{

  init();
  
#if 1  
  mug_motion_on(motion_handle, on_motion);
  mug_motion_angle_on(motion_handle, on_angle);
  mug_set_motion_timer(motion_handle, 1000);
  //mug_run_touch_thread();
  mug_run_motion_watcher(motion_handle);
#else
  while(1) {
    mug_read_motion(handle, &data);
    printf("      ax:%8d, ay:%8d, az:%8d, gx:%8d, gy:%8d, gz:%8d\n",
           data.ax, data.ay, data.az, data.gx, data.gy, data.gz);
    usleep(500*1000);
  }
#endif
  return 0;
}

