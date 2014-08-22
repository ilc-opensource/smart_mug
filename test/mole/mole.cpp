#define cimg_display 0 
#include <stdlib.h>
#include <pthread.h>
#include "CImg.h"
using namespace cimg_library;

#include <list>
#include <algorithm>
using namespace std;

#if !cimg_display
#include <mug.h>
#endif

#define ROUND_NUM   10
#define SCR_WIDTH   16
#define SCR_HEIGHT  12

#define MOLE_COL    4
#define MOLE_ROW    3

#define MOLE_MAX    (MOLE_COL * MOLE_ROW)

#define MOLE_WIDTH  (SCR_WIDTH  / MOLE_COL)
#define MOLE_HEIGHT (SCR_HEIGHT / MOLE_ROW)

#define MOLE_PIC      "mole.bmp"
#define MOLE_HIT_PIC  "mole_hit.bmp"

typedef unsigned char * color_t;

CImg<unsigned char> canvas(SCR_WIDTH, SCR_HEIGHT, 1, 3, 0);
CImg<unsigned char> mole_pic(MOLE_PIC);
CImg<unsigned char> mole_hit_pic(MOLE_HIT_PIC);

class mole_t 
{
  public:
    int idx;
    int col, row;
    int width, height;
    bool touched;

    mole_t(int i);
    mole_t(int c, int w);
};

typedef list<mole_t> mole_list_t;

mole_t::mole_t(int i)
{
  idx = i;
  width = MOLE_WIDTH;
  height = MOLE_HEIGHT;
  col = idx % MOLE_COL * MOLE_WIDTH;
  row = idx / MOLE_COL * MOLE_HEIGHT;
  touched = false;
}

mole_t::mole_t(int c, int w)
{
  idx = -1;
  width = mole_pic.width();
  height = mole_pic.height();
  
  col = c;
  row = w;
}

class score_t
{
  public:
    int touched;
    int all;
    void reset();
};

void score_t::reset()
{
  touched = 0;
  all = 0;
}

pthread_mutex_t mutex;

void my_lock() 
{
  static int i = 0;
  pthread_mutex_lock(&mutex);
  printf("locked %d\n", i++);
}

void my_unlock() 
{
  static int i = 0;
  pthread_mutex_unlock(&mutex);
  printf("unlocked %d\n", i++);
}

#define LOCK_  pthread_mutex_lock(&mutex)
#define UNLOCK_  pthread_mutex_unlock(&mutex)
//#define LOCK_ my_lock()
//#define UNLOCK_ my_unlock()
mole_list_t mole_list;
score_t     score;
int         round_num = ROUND_NUM;
#if !cimg_display
handle_t    handle;
#endif

void draw_mole(mole_t *mole, CImg<unsigned char> *pic) 
{
  canvas.draw_image(mole->col, mole->row, 0, 0,
                    *pic);
}

void draw_all(CImg<unsigned char> *pic)
{
  for(mole_list_t::iterator itr = mole_list.begin();
      itr != mole_list.end();
      itr++) {
    draw_mole(&(*itr), pic);
  }
}

void save_canvas(int i)
{
  char fname[256];
  
  sprintf(fname, "save_%d.bmp", i);

  canvas.save(fname);
}

#if cimg_display
void disp_canvas()
{
  CImgDisplay draw_disp(canvas, "mole");

  while(!draw_disp.is_closed()) {
    draw_disp.wait();
  }
}
#else
void disp_canvas()
{
  mug_disp_cimg(handle, &canvas); 
}
#endif


void clear_canvas()
{
  canvas.draw_rectangle(0, 0, 0,
                        SCR_WIDTH, SCR_HEIGHT, 0,
                        black);

}

mole_t* is_in(int c, int r)
{
  for(mole_list_t::iterator itr = mole_list.begin();
      itr != mole_list.end();
      itr++) {
    if(   (*itr).col <= c && c < (*itr).col + (*itr).width
       && (*itr).row <= r && r < (*itr).row + (*itr).height) {
      return &(*itr);
    }
  }

  return NULL;
}

void gen_round_mole(int *x, int *y)
{
}

void next_round(int num)
{
  LOCK_;

  printf("-------\n");
  clear_canvas();
  mole_list.clear();
  score.all += num;
  
  int col = rand() % (SCR_WIDTH  - mole_pic.width());
  int row = rand() % (SCR_HEIGHT - mole_pic.height());

  for(int i = 0; i < num; i++) {
    while(is_in(col, row)) {
      col = rand() % (SCR_WIDTH  - mole_pic.width());
      row = rand() % (SCR_HEIGHT - mole_pic.height());
    }
    printf("(%d, %d)\n", col, row);
    mole_t mole(col, row);
    mole_list.push_back(mole);  
  }

  draw_all(&mole_pic);

  disp_canvas();

  UNLOCK_;
}

void show_result()
{
  printf("%d / %d\n", score.touched, score.all);

  clear_canvas();

  int width, height;
  mug_number_text_shape(&width, &height);

  char temp[4];
  sprintf(temp, "%02d", score.touched);
  mug_draw_number_cimg(&canvas, 0, 0, temp, red);

  sprintf(temp, "%02d", score.all);
  mug_draw_number_cimg(&canvas, width * 2 , 0, temp, green);

  disp_canvas();
}

void touch_on(int x, int y, int id)
{
  if(id != 0) return;
  LOCK_;

  mole_t *mole;
  bool changed = false;

  mole = is_in(x, y);

  if(mole && !mole->touched) {
    changed = true;
    mole->touched = true;
    score.touched++;
    printf("(%d, %d) -> (%d, %d)\n", x, y, mole->col, mole->row);
    draw_mole(mole, &mole_hit_pic);
  }

  if(changed)
    disp_canvas();

  UNLOCK_;
}

void *touch_thread(void *arg)	
{
  mug_run_touch_thread();
}

void init()
{
  pthread_mutex_init(&mutex, NULL);

#if !cimg_display
  handle = mug_disp_init();
  mug_touch_on(touch_on);

#ifdef USE_LIBUV  
  pthread_t hdl;
  pthread_create(&hdl, NULL, touch_thread, NULL);
#else
  mug_run_touch_thread();
#endif

#endif

}

void finish()
{
  mug_stop_touch_thread();
  pthread_mutex_destroy(&mutex);
}

int main(int argc, char **argv)
{
  init();
 
  for(int i = 0; i < round_num; i++) {
    next_round(2);
    usleep(1000*1000);
  }

  show_result();

  finish();
}
