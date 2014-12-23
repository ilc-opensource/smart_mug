#define cimg_display 0
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mug.h>

#include <string>
using namespace std;

#include <CImg.h>
using namespace cimg_library;

CImg<unsigned char> canvas(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 3, 0);

#define WARM 30
#define HOT  50
#define BAR_NUM 10
#define BAR_STEP (100 / BAR_NUM)

handle_t disp_handle;
handle_t battery_handle;

void clear_canvas()
{
  canvas.draw_rectangle(0, 0, 0,
                        SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                        black);

}

void draw_data(int t, char *c)
{
  clear_canvas();
 
  char temp[5];

  sprintf(temp, "%d", t);
   
  canvas.draw_text(0, 0, temp, color_to_rgb(c), black);

  mug_disp_cimg(disp_handle, (cimg_handle_t)&canvas);
}

void draw_bar(int percent, int is_charge)
{ 
  int normal = (percent - (percent % BAR_STEP));

  printf("normal to %d\n", normal);
  
  char f[128];

  sprintf(f, "%s_%02d.bmp", is_charge ? "charge" : "discharge", normal);

  mug_disp_img(disp_handle, f);
}

void on_battery(int percent, int is_charge)
{
  printf("%s: %02d\n", is_charge? "charge": "discharge", percent);
#if 0
  printf("%d\n", percent);
  if(is_charge)
    draw_data(percent, "green");
  else
    draw_data(percent, "magenta");
#else
  draw_bar(percent, is_charge);
#endif
}

int main(int argc, char** argv)
{
  printf("init ok\n");
  disp_handle = mug_disp_init();
  battery_handle = mug_battery_init();

  mug_battery_on(battery_handle, on_battery, 1000);
  mug_run_battery_watcher(battery_handle);

  return 0;
}
