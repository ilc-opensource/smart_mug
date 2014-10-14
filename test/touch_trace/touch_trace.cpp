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
#define MAX_TRACES 5

CImg<unsigned char> canvas(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 3, 0);

handle_t touch_handle;
handle_t disp_handle;

static char *raw_buf = NULL;
unsigned char colors[MAX_TRACES];

typedef struct _last_touch {
  int x, y, id;
} last_touch;

last_touch last[MAX_TRACES];

void on_touch(int x, int y, int id)
{
  MUG_ASSERT(id < MAX_TRACES, "exceed max_trace");

  last_touch *l = &(last[id]);
  
  printf("%d: [%4d, %4d]", id, x, y);

  if(l->x != x || l->y != y) {
    mug_set_pixel_raw_color(raw_buf, x, y, colors[id]);
    mug_disp_raw(disp_handle, raw_buf);
    l->x = x;
    l->y = y;
    l->id = id;
    printf("\n");
  } else {
    printf("skipped\n");
  }
}

void clear_canvas()
{
  memset(raw_buf, 0, COMPRESSED_SIZE);
  printf("canvas cleared\n");  
}

void on_touch_event(touch_event_t e, int x, int y, int id)
{
  printf("event: %d\n", e);
  clear_canvas();
}

void init()
{
  colors[0] = color_2_raw(green);
  colors[1] = color_2_raw(yellow);
  colors[2] = color_2_raw(magenta);
  colors[3] = color_2_raw(cyan);
  colors[4] = color_2_raw(blue);

  memset(last, -1, sizeof(last));
}

int main()
{
  // init
  touch_handle = mug_touch_init();
  disp_handle = mug_disp_init();
  init();

  raw_buf = mug_create_raw_buffer();
  clear_canvas();
  canvas.draw_text(0, 0, "ok", cyan, black);
  mug_disp_cimg(disp_handle, (cimg_handle_t)&canvas);
  usleep(1000 * 1000);
  canvas.draw_rectangle(0, 0, 0,
                        SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                        black);
  mug_disp_cimg(disp_handle, (cimg_handle_t)&canvas);

  mug_disp_raw(disp_handle, raw_buf);
  mug_touch_on(touch_handle, on_touch);
  mug_touch_event_on(touch_handle, TOUCH_UP, on_touch_event);
  mug_run_touch_thread(touch_handle);

}
