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
#define INTERVAL 1000

handle_t disp_handle;
handle_t battery_handle;

int last_stat = -1;
int last_percent = -1;
int tolerant_diff = 10;
int counter = 0;
int max_counter = (30 * 1000) / INTERVAL;

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
  printf("draw: %s: %02d\n", is_charge? "charge": "discharge", normal);
  
  char f[128];

  sprintf(f, "%s_%02d.bmp", is_charge ? "charge" : "discharge", normal);

  mug_disp_img(disp_handle, f);
}

#if 0
void on_battery(int percent, int is_charge)
{
  printf("recv: %s: %02d\n", is_charge? "charge": "discharge", percent);

  int diff;

  if(last_stat < 0) {
    last_percent = percent;
    last_stat = is_charge;
  }

  if(last_stat != is_charge) {
    printf("*** changed ***\n");
    diff = abs(last_percent - percent);
    if(diff < tolerant_diff) {
      printf("OK, change state\n");
      last_percent = percent;
      last_stat = is_charge;
    } else {
      printf("diff %d >＝ %d, use old data\n", diff, tolerant_diff);
    }
  } else {
    last_percent = percent;
  }

  printf("disp: %s: %02d\n", is_charge? "charge": "discharge", last_percent);

  draw_bar(last_percent, is_charge);

  printf("\n");
}
#else
void on_battery(int percent, int is_charge)
{
  printf("\nrecv: %s: %02d\n", is_charge? "charge": "discharge", percent);
  
  if(last_stat < 0 ) {
    last_stat = is_charge;
    last_percent = percent;
  }

  // when charge to discharge, reset counter;
  if(last_stat != is_charge && !is_charge) {

    if(counter == 0) {
      printf("--> charge to discharge, start max_counter with %d\n", max_counter);
      counter = max_counter;
      draw_bar(last_percent, is_charge);
    } 

  } else {
    if(counter > 0) {
      printf("--> reset counter\n");
      counter = 0;    
    }
  }  

  if(counter > 0) {
    printf("--> counter is %d, hang up\n", counter);
    counter--; 

    if(counter == 0) {
      printf("--> timer stopped\n");
      last_stat = is_charge;
    }   

    return;
  }
  


  last_stat = is_charge;
  last_percent = percent;

  draw_bar(last_percent, is_charge);  
}

#endif
int main(int argc, char** argv)
{
#if 0
  if(argc > 1) {
    tolerant_diff = atoi(argv[1]);
  }

  printf("tolerant_diff is %d\n", tolerant_diff);
#else
  if(argc > 1) {
    max_counter = (atoi(argv[1]) * 1000 + INTERVAL - 1) / INTERVAL;
  }
  printf("set max_counter counter to: %d\n", max_counter);
#endif
  disp_handle = mug_disp_init();
  battery_handle = mug_battery_init();

  mug_battery_on(battery_handle, on_battery, INTERVAL);
  mug_run_battery_watcher(battery_handle);

  return 0;
}
