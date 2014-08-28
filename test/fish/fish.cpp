#define cimg_display 0 
#include <stdlib.h>
#include <pthread.h>
#include "CImg.h"
using namespace cimg_library;

#include <list>
#include <vector>

using namespace std;

#if !cimg_display
#include <mug.h>
#endif

#ifdef USE_LIBUV
#include <uv.h>
#endif

const char* fish_pics[] = {"fish0.bmp", "fish1.bmp"};
#define fish_pics_num (sizeof(fish_pics) / sizeof(const char*))

#define FISH_PIC "fish.bmp"

#define INTERVAL 500
#define TIMES_PER_SEC (1000/INTERVAL)
#define DEFAULT_HEALTH 12
#define DEFAULT_SPEED 1
#define GZ_THRESH 5

#define TEMP_WARM 30
#define TEMP_HOT  50

typedef struct fish_t_ {

  int x, y;
  int x_speed, y_speed;
  int health;
  int pic_idx;

  unsigned char *color;
  CImg<unsigned char> *img_right;
  CImg<unsigned char> *img_left;
  CImg<unsigned char> *img;

} fish_t;

handle_t disp_handle;
handle_t motion_handle;
handle_t touch_handle;
handle_t temp_handle;

CImg<unsigned char> canvas(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 3, 0);
fish_t *fish = NULL;

static void normalize_color(CImg<unsigned char> &img)
{
  for(int r = 0; r < img.height(); r++) {
    for(int c = 0; c < img.width(); c++) {
      for(int color = 0; color < 3; color++) {
        if(img(c, r, 0, color) < 128) {
          img(c, r, 0, color) = 0;
        } else {
          img(c, r, 0, color) = 255;
        }
      }
    }
  }
}

void clear_canvas()
{
  canvas.draw_rectangle(0, 0, 0,
                        SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                        black);

}

void change_img_color(CImg<unsigned char> &img, unsigned char *color)
{
  for(int r = 0; r < img.height(); r++) {
    for(int c = 0; c < img.width(); c++) {
      // skip black background
      if(img(c, r, 0, 0) == 0 
         && img(c, r, 0, 1) == 0 
         && img(c, r, 0, 2) == 0)
      continue;

        img(c, r, 0, 0) = color[0];
        img(c, r, 0, 1) = color[1];
        img(c, r, 0, 2) = color[2];
      }
   }
}

void change_fish_color(fish_t *f, unsigned char* color)
{
  change_img_color(*(f->img_right), color);
  change_img_color(*(f->img_left), color);
}

void set_direction(fish_t *f)
{
  if(f->x_speed > 0) {
    f->img = f->img_right;
  } else if(f->x_speed < 0) {
    f->img = f->img_left;
  }

}

void load_fish_pic(fish_t *f)
{
  f->img_right = new CImg<unsigned char>(fish_pics[f->pic_idx]);
  f->img_left = new CImg<unsigned char>(fish_pics[f->pic_idx]);	
  f->img_left->mirror("x");
  set_direction(f);

  normalize_color(*(f->img_right));
  normalize_color(*(f->img_left));

  change_fish_color(f, f->color);

}

fish_t* new_fish()
{
  fish_t *f = (fish_t*)malloc(sizeof(fish_t));
  memset(f, 0, sizeof(fish_t));
 
  f->x_speed = DEFAULT_SPEED;
  f->y_speed = DEFAULT_SPEED;

  f->health = DEFAULT_HEALTH;
  f->color = green;

  f->pic_idx = 0;

  load_fish_pic(f);
  
  return f;
}

void change_fish_pic(fish_t *f)
{
  free(f->img_right);
  free(f->img_left);

  f->pic_idx = (f->pic_idx + 1) % (fish_pics_num);
  load_fish_pic(f);
}

void draw_fish(fish_t *f)
{
  canvas.draw_image(f->x, f->y, 0, 0,
                    *(f->img));
}

void fish_swim(fish_t *f)
{
  if(f->health <= 0) {
    f->health = 0;
    f->x_speed = 0;
    f->y_speed = 0;
  }
 
  // If fish has not engergy, just sink down
  if(f->x_speed == 0 && f->y_speed == 0) {
    if(f->y < (SCREEN_HEIGHT - f->img->height()))
      f->y++;
    return;
  }

  int new_x = f->x + f->x_speed;
  if(new_x+ f->img->width() >= SCREEN_WIDTH
     || new_x < 0) {
    f->x_speed = 0 - f->x_speed;
  }

  int new_y = f->y + f->y_speed;
  if(new_y+ f->img->height() >= SCREEN_HEIGHT
     || new_y < 0) {
    f->y_speed = 0 - f->y_speed;
  }

  f->x += f->x_speed;
  f->y += f->y_speed;

  set_direction(f);

  f->health -= (abs(f->x_speed) + abs(f->y_speed));

}

void resize_fish(fish_t *f)
{
  f->img_right->resize(5, 4);
  f->img_left->resize(5, 4);
}

void disp_canvas()
{
  mug_disp_cimg(disp_handle, &canvas);
}

void on_motion(int ax, int ay, int az, int gx, int gy, int gz)
{
  printf("async ax:%8d, ay:%8d, az:%8d, gx:%8d, gy:%8d, gz:%8d\n",
         ax, ay, az, gx, gy, gz);

  int norm_gz = (abs(gz) + (131/2) ) / 131;
  
   if(norm_gz > GZ_THRESH) {
    fish->x_speed = DEFAULT_SPEED;
    fish->y_speed = DEFAULT_SPEED;
    fish->health = DEFAULT_HEALTH;
   }
}

void on_temp(int board_temp, int mug_temp, int battery_temp)
{
  printf("board temp: %d\n", board_temp);

  if(board_temp < TEMP_WARM) {
    change_fish_color(fish, blue);
    fish->color = blue;

  } else if( TEMP_WARM <= board_temp && board_temp < TEMP_HOT) {
    change_fish_color(fish, yellow);
    fish->color = yellow;

  } else {
    change_fish_color(fish, red);
    fish->color = red;
  }
}

void on_click(touch_event_t e, int x, int y, int z)
{
  printf("click @ (%d, %d, %d)\n", x, y, z);
  if(fish->x <= x && x < fish->x + fish->img->width()
     && fish->y <= y && y < fish->y + fish->img->height()) {
    printf("HIT!!!!!\n");
    change_fish_pic(fish);
  }
}

void init()
{
  disp_handle   = mug_disp_init();
  motion_handle = mug_motion_init();
  touch_handle  = mug_touch_init();
  temp_handle   = mug_temp_init();

  fish = new_fish();

}

void handle_fish()
{
  draw_fish(fish);
  fish_swim(fish);
  disp_canvas();
  clear_canvas();
}

#ifdef USE_LIBUV
static uv_loop_t *fish_loop = NULL;
static uv_timer_t fish_timer;

void run_fish_timer(uv_timer_t *req, int status)
{
  handle_fish();
}

int main()
{
  init();

  // main loop
  fish_loop = uv_default_loop();
  uv_timer_init(fish_loop, &fish_timer);
  uv_timer_start(&fish_timer, run_fish_timer, 0, INTERVAL);

  // read motion
  mug_motion_on(motion_handle, on_motion,  200);

  // read temprature
  mug_temp_on(temp_handle, on_temp, 1000);

  // read click
  mug_touch_event_on(touch_handle, TOUCH_CLICK, on_click);
  
  mug_run_motion_watcher(motion_handle);
}

#endif

