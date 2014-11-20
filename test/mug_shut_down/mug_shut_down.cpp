#define cimg_display 0 
#include <stdlib.h>
#include "CImg.h"
using namespace cimg_library;

#include <mug.h>

#include <string>
using namespace std;

#define DELAY   10
#define CMD_YES "./run_shut_down.sh"
#define CMD_NO  "./abort_shut_down.sh"
#define PIC_YES "yes.bmp"
#define PIC_NO  "no.bmp"

handle_t disp_handle;
handle_t touch_handle;

static int delay = DELAY;

CImg<unsigned char> canvas(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 3, 0);

class Button {
public:
  Button(int x, int y, char *pic);
  CImg<unsigned char> cimg;
  int x, y;
  bool is_clicked(int cx, int cy); 
};

Button *yes = NULL;
Button *no = NULL;

Button::Button(int x, int y, char *pic):
  cimg(pic), x(x), y(y) {
 
}

bool Button::is_clicked(int cx, int cy) 
{
  return( x <= cx && cx < x + cimg.width() &&
          y <= cy && cy < y + cimg.height());
}

void update_canvas()
{
  canvas.draw_image(yes->x, yes->y, 0, 0, yes->cimg);
  canvas.draw_image(no->x, no->y, 0, 0, no->cimg);
}

void on_touch_event(touch_event_t e, int x , int y, int id)
{
  if(yes->is_clicked(x, y)) {
    printf("YES\n");
    printf("shut down signal after %d\n", delay);
    mug_disp_text_marquee(disp_handle, "shutting down...", "green", 100, 1); 
    mug_shut_down_mcu(delay);
    printf("run %s\n", CMD_YES);
    system(CMD_YES);

    exit(0);
  } else if(no->is_clicked(x, y)) {
    printf("NO\n");
    printf("%s\n", CMD_NO);
    mug_disp_text_marquee(disp_handle, "cancelled", "red", 100, 1); 
    printf("run %s\n", CMD_NO);
    exit(1);
  }
}

int main(int argc, char** argv)
{
  if(argc > 1) {
    delay = atoi(argv[1]);
  };
  printf("delay %d\n", delay);

  yes = new Button(0, 0, PIC_YES);
  no =  new Button(yes->cimg.width(), 0, PIC_NO);
  update_canvas();

  disp_handle = mug_disp_init();
  touch_handle = mug_touch_init();
  mug_init_font(NULL);
  mug_disp_text_marquee(disp_handle, "shut down ?", "red", 100, 1);  

  mug_disp_cimg(disp_handle, (cimg_handle_t)&canvas);
  mug_touch_event_on(touch_handle, TOUCH_CLICK, on_touch_event);
  mug_run_touch_thread(touch_handle);
  return 0;
}
