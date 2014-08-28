#define cimg_display 0
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mug.h>
#include <CImg.h>
using namespace cimg_library;

#include <list>
using namespace std;

#define MAX_NUM 4

CImg<unsigned char> canvas(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 3, 0);

handle_t touch_handle;
handle_t disp_handle;

void on_touch(int x, int y, int id)
{
  printf("%d: [%4d, %4d]\n", id, x, y);
  unsigned char* color = green;
  if(id == 1)
    color = yellow;

  canvas.draw_point(x, y, 0, color);

  mug_disp_cimg(disp_handle, &canvas);
}

void on_touch_event(touch_event_t e, int x, int y, int id)
{
  printf("event: %d\n", e);
  canvas.draw_rectangle(0, 0, 0,
                        SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                        black);
}

int main()
{
  touch_handle = mug_touch_init();
  disp_handle = mug_disp_init();

  mug_touch_on(touch_handle, on_touch);
  mug_touch_event_on(touch_handle, TOUCH_UP, on_touch_event);
  mug_run_touch_thread(touch_handle);
}
