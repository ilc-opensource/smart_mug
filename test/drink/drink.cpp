#define cimg_display 0
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mug.h>
#include <time.h>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <string>
#include <list>
using namespace std;

#include <CImg.h>
using namespace cimg_library;

#define DIR "/home/root/drink_trace"
#define CONFIG "./drink_config.json"
#define PREFIX "drink"
#define LEAST_TIME 2

static bool disp = false;
static bool touch = false;

static FILE *fp = NULL;

typedef struct _config_t {
  time_p start_time

};

typedef list<int> degree_list_t;

class trace_t {
public:
  trace_t();
  int id;
  degree_list_t degree_list;
  bool catch_it;
  int drink_deg;
  time_t begin_time, end_time;  
  int dur_time;
  bool is_drinking;
};

trace_t::trace_t()
  :id(-1), begin_time(0), end_time(0), is_drinking(false), catch_it(false), drink_deg(0)
{
  degree_list.clear();
}

CImg<unsigned char> canvas(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 3, 0);

#define WARM 30
#define HOT  50

handle_t disp_handle;
handle_t motion_handle;
handle_t touch_handle;

trace_t trace;

static int trigger_deg = 80;
static int rec_deg = 90;

void clear_canvas()
{
  canvas.draw_rectangle(0, 0, 0,
                        SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                        black);

}

void draw_data(int t, char *color)
{
  clear_canvas();
 
  char temp[5];

  sprintf(temp, "%d", t);

  canvas.draw_text(0, 0, temp, cyan, black);

  mug_disp_cimg(disp_handle, (cimg_handle_t)&canvas);
}

void disp_start()
{
  clear_canvas();
  canvas.draw_rectangle(0, 0, 0,
                        SCREEN_WIDTH/4, SCREEN_HEIGHT/4, 0,
                        green);
  mug_disp_cimg(disp_handle, (cimg_handle_t)&canvas);
}

void disp_stop()
{
  clear_canvas();
  canvas.draw_rectangle(0, 0, 0,
                        SCREEN_WIDTH/4, SCREEN_HEIGHT/4, 0,
                        red);
  mug_disp_cimg(disp_handle, (cimg_handle_t)&canvas);
}

void dump_trace(FILE *f)
{
  struct tm *begin_tm, *end_tm;
  begin_tm = localtime(&trace.begin_time);
  end_tm = localtime(&trace.end_time);

  fprintf(f, "  {\n");
  fprintf(f, "    \"id\": %d,\n", trace.id);
  fprintf(f, "    \"drink_deg\": %d,\n", trace.drink_deg);
  fprintf(f, "    \"beginTime\": \"%d_%02d_%02d %02d:%02d:%02d\",\n",
             begin_tm->tm_year + 1900, begin_tm->tm_mon + 1, begin_tm->tm_mday, begin_tm->tm_hour, begin_tm->tm_min, begin_tm->tm_sec);
  fprintf(f, "    \"duration\" : %d,\n", trace.dur_time);

  fprintf(f, "    \"trace\": [ ");

  string str;
  
  degree_list_t::iterator last_itr = trace.degree_list.end();
  last_itr--;

  for(degree_list_t::iterator itr = trace.degree_list.begin();
      itr != trace.degree_list.end();
      itr++) {
    char temp[16];

    if(itr != last_itr)
      sprintf(temp, " %d,", *itr);      
    else
      sprintf(temp, " %d", *itr);
    str += temp; 
  }

  fprintf(f, "%s ]\n", str.c_str());
  fprintf(f, "  },\n");

}

void read_config()
{
  FILE *fp = fopen(CONFIG, "r");
  
  MUG_ASSERT(fp, "can not find drink config file: %s\n", CONFIG);

    // check file length
  fseek(fp,0,SEEK_END);
  long len=ftell(fp);
  fseek(fp,0,SEEK_SET);

  char *data=(char*)malloc(len+1);
  fread(data,1,len,fp);
  fclose(fp);

  cJSON json = cJSON_Parse(data);

  free(data);
}

void init_file()
{
  time_t raw_time;
  struct tm *timeinfo;

  time(&raw_time);
  timeinfo = localtime(&raw_time);

  char temp[128];

  sprintf(temp, "%s/%s_%02d_%02d_%02d%02d%02d", 
                DIR, PREFIX, 
                timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min);
  
  fp = fopen(temp, "w+");
  
  MUG_ASSERT(fp != NULL, "can not open %s", temp);

  fprintf(fp, "{\n");  

}

void clear_trace()
{
  trace.degree_list.clear();
  trace.drink_deg = 0;
  trace.dur_time = 0;
  trace.is_drinking = false;

}

void start_trace()
{
  clear_trace();
  disp_start();
  trace.is_drinking = true;
  trace.id++;

  time(&trace.begin_time);
  time(&trace.end_time);
}

void stop_trace()
{
  disp_stop();
  trace.is_drinking = false;
  time(&trace.end_time);
  trace.dur_time = trace.end_time - trace.begin_time;

  MUG_ASSERT(trace.dur_time >= 0, "drink time < 0");

  if(trace.dur_time < LEAST_TIME) {
    printf("abondon trace due to duration time %d < %d\n",trace.dur_time, LEAST_TIME);
    clear_trace();
    return;
  }

  if(trace.drink_deg == 0) {
    printf("didn't capture dring degree\n");
    clear_trace();
    return;
  }

  dump_trace(fp);
  dump_trace(stdout);
}

void on_signal(int signo) 
{
  printf("captured ctrl + c\n");
  fprintf(fp, "}\n");
  fclose(fp);
  exit(0);
}

void on_motion(int ax, int ay, int az, int gx, int gy, int gz)
{
  printf("%6d, %6d, %6d, %6d, %6d, %6d\n", ax, ay, az, gx, gy, gz);
}

void on_motion_angle(float angle_x, float angle_y, float angle_z)
{
  //printf("%4.2f, %4.2f, %4.2f\n", angle_x, angle_y, angle_z);
  
  if(trace.catch_it) {
    trace.drink_deg = (int)angle_z;
    draw_data((int)angle_z, "magenta");
    trace.catch_it = false;
  } 

  if(angle_z <= trigger_deg) {
    if(!trace.is_drinking) {
      printf("start drink, when > %d\n", trigger_deg);
      start_trace();
    }
  } else {
    if(trace.is_drinking) {
      printf("stop drink\n");
      stop_trace();
    }
  }

  if(trace.is_drinking) {
    trace.degree_list.push_back(angle_z);
  }
}

void on_touch(touch_event_t e, int x, int y, int id)
{
  if(e == TOUCH_DOWN) {
    printf("catched\n");
    trace.catch_it = true;
  }
}

int main(int argc, char** argv)
{

  int c;
  while((c = getopt(argc, argv, "pth")) != -1) {
    switch(c) {
    case 'p':
      disp = true;
      break;
    case 't':
      touch = true;
      break;
    }
  }

  if(disp) {
    disp_handle = mug_disp_init();
  }
  
  if(touch) {
    touch_handle = mug_touch_init();
  }
  
  motion_handle = mug_motion_init();

  init_file();

  signal(SIGINT, on_signal);

  mug_touch_event_on(touch_handle, TOUCH_EVENT_ALL, on_touch);
  mug_motion_angle_on(motion_handle, on_motion_angle);
  mug_set_motion_timer(motion_handle, 100);
  
  mug_run_motion_watcher(motion_handle);

  return 0;
}
