#include <stdlib.h>
#include <dirent.h>

#include <string>
#include <vector>

#include <mug.h>
using namespace std;

typedef vector<string> name_list_t;

name_list_t file_list;
int idx = 0;
int hit_idx = -1;


int vol = 5;

#define VOL_MAX 16
#define VOL_MIN 0
#define VOL_STEP 2

handle_t disp_handle;
handle_t touch_handle;

#define HIT "magenta"
#define IDLE "cyan"

#define CMD "cat"
#define OPTION "/dev/pts/1"

void run_player(char *option)
{
  printf("run player %s\n", option);
  string str(CMD);
  str += " ";
  str += OPTION;
  str += " &";
  system(str.c_str());
}

void stop_player()
{
  printf("stop player\n");
  string str("killall ");
  str += CMD;
  system(str.c_str());
}

bool filter_file(string &str)
{
  if(str == ".." || str == ".")
    return true;

  size_t ext_idx = str.rfind('.');
  if(ext_idx == string::npos)
    return true;
  
  string ext = str.substr(ext_idx + 1);
  if(!(ext == "wav" || ext == "WAV" || ext == "mp3"))
    return true;

  return false;
}

void scan_files(const char* target)
{
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir (target)) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {

      string str(ent->d_name);

      if(filter_file(str))
        continue;

      printf ("%s\n", ent->d_name);
      file_list.push_back(str);
    }
    closedir (dir);
  } else {
    MUG_ASSERT(false, "can not read directory %s\n", target);
  }
}

void disp_curr_file()
{
  mug_disp_text_marquee_async(disp_handle, file_list[idx].c_str(), idx == hit_idx ? HIT : IDLE, 100, DISP_INFINITE);
}

void on_touch_event(touch_event_t e, int x, int y, int id)
{
  bool player_running = false;

  //printf("before idx: %d, hit: %d\n", idx, hit_idx);

  if(hit_idx >= 0) {
    stop_player();
  }

  if(hit_idx < 0 || hit_idx != idx) {
    run_player(file_list[idx].c_str());
    hit_idx = idx;
  } else {
    hit_idx = -1;
  }

  fflush(0);

  //printf("after idx: %d, hit: %d\n", idx, hit_idx);  
  disp_curr_file();
}

void disp_vol()
{
  char buf[16];
  sprintf(buf, "%d", vol);
  mug_disp_text_marquee(disp_handle, buf, "yellow", 200, 1);
}

void volumn_up()
{
  vol += VOL_STEP;
  if(vol > VOL_MAX)
    vol = VOL_MAX;

  disp_vol();
}

void volumn_down()
{
  vol -= VOL_STEP;
  if(vol < 0)
    vol = VOL_MIN;

  disp_vol();
}

void on_gesture(gesture_t g, char* info)
{
  int size = file_list.size();

  if(g == MUG_SWIPE_LEFT) {
    idx--;
    if(idx < 0)
      idx = size - 1;
  } else if(g == MUG_SWIPE_RIGHT) {
    idx++;
    idx = idx % size;
  } else if(g == MUG_SWIPE_DOWN) {
    volumn_down();
  } else if(g == MUG_SWIPE_UP) {
    volumn_up();
  }
  disp_curr_file();
}

int main(int argc, char** argv)
{
  stop_player();

  if(argc != 2) {
    printf("must specify target directory\n");
    return 1;
  }
    
  char *target = argv[1];
  scan_files(target);

  mug_init_font(NULL);

  disp_handle  = mug_disp_init();
  touch_handle = mug_touch_init(); 
  
  int size = file_list.size();
  if(size == 0) {
    mug_disp_text_marquee(disp_handle, "no files", "red", 100, DISP_INFINITE);
  } else {
    mug_touch_event_on(touch_handle, TOUCH_CLICK, on_touch_event);
    mug_gesture_on(touch_handle, MUG_GESTURE, on_gesture);  
    mug_disp_text_marquee_async(disp_handle, file_list[idx].c_str(), IDLE, 100, DISP_INFINITE);
    mug_run_touch_thread(touch_handle);
  }
 
  return 0;
}
