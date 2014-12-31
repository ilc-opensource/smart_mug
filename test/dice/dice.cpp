#define cimg_display 0 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mug.h>
#include <CImg.h>
using namespace cimg_library;

static handle_t disp_handle;
static handle_t motion_handle;

CImg<unsigned char> canvas(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 3, 0);

void clear_canvas()
{
  canvas.draw_rectangle(0, 0, 0,
                        SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                        black);

}

void on_motion(int ax, int ay, int az, int gx, int gy, int gz)
{
  printf("async ax:%8d, ay:%8d, az:%8d, gx:%8d, gy:%8d, gz:%8d\n",
          ax, ay, az, gx, gy, gz);
 
}

int gen_rand()
{
  time_t now;
  time(&now);

  srand(now & 0xffffFFFF);

  return (rand() % 6 + 1);
}

void on_shake(int start)
{
  char buf[32];

  if(start) {
    printf("===> shake start!\n");
    mug_disp_img(disp_handle, "busy.bmp");
  } else {
    printf("===> shake stop!\n");
    sprintf(buf, "dice_%d.bmp", gen_rand());
    mug_disp_img(disp_handle, buf);
  }
}

void init()
{
  disp_handle = mug_disp_init();
  motion_handle = mug_motion_init();
}

int main(int argc, char** argv)
{

  init();
  
  mug_set_text_marquee_style(MQ_PROLOG);
  mug_disp_text_marquee_async(disp_handle, "shake!!", "magenta", 70, 1);

  //mug_motion_on(motion_handle, on_motion);
  mug_motion_shake_on(motion_handle, on_shake);

  mug_config_shake(motion_handle, 500, 1);

  //mug_set_motion_timer(motion_handle, 100);
  mug_run_motion_watcher(motion_handle);

  return 0;
}

