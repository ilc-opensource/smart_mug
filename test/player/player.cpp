#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <iostream>

#include <mug.h>
#include <config.h>
using namespace std;

typedef vector<string> name_list_t;

name_list_t file_list;
int idx = 0;
int hit_idx = -1;


int vol = 50;

#define VOL_MAX 99
#define VOL_MIN 0
#define VOL_STEP 10

handle_t disp_handle;
handle_t touch_handle;

#define HIT  "green"
#define IDLE "cyan"
#define REP  "###"

static const char *player_bin = NULL;
static const char *player_option = NULL;
static const char *vol_control = NULL;
static const char *target = NULL;
static const char *kill_cmd = NULL;
static const char *run_cmd = NULL;

#define RUN_PLAYER "./run_player.js"
#define STOP_PLAYER "./stop_player.sh"

static pid_t pid = 0;
 
void replace(string &str, string src, string repl)
{
  size_t pos = str.find(src);  
  str.erase(pos, src.length());
  str.insert(pos, repl);
}

#if 0
void run_player(char *option)
{
  string str(player_bin);
  string rep(REP);

  str += " ";
  str += player_option;
  str += " &";

  replace(str, rep, option);
  
  string run_str(run_cmd);
  replace(run_str, REP, str);

  printf("run: %s\n", run_str.c_str());

  system(run_str.c_str());
}
void stop_player()
{  
  string str(kill_cmd);
  replace(str, REP, player_bin);
  printf("%s\n", str.c_str());

  system(str.c_str());
}
#else

void run_player(char *option) 
{
  string str(RUN_PLAYER);
  char temp[10];
  sprintf(temp, "%d", pid);

  str += " \"";
  str += option;
  str += "\" ";
  str += temp;
  str += " &";

  printf("%s\n", str.c_str());
  system(str.c_str());
}

void stop_player() 
{
  string cmd(STOP_PLAYER);
  system(cmd.c_str());
}

#endif

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

  string f;
  if(hit_idx < 0 || hit_idx != idx) {
    hit_idx = idx;
    disp_curr_file();

    f = target;
    f += "/";
    f += file_list[idx];
    run_player(f.c_str());
  } else {
    hit_idx = -1;
    disp_curr_file();
  }

  fflush(0);
  //printf("after idx: %d, hit: %d\n", idx, hit_idx);  
}

void disp_vol()
{  
  char buf[16];
  sprintf(buf, "%d", vol);

  string str(vol_control);
  replace(str, REP, buf);
  printf("%s\n", str.c_str());
  system(str.c_str());

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

void on_signal(int signo) 
{
  if(signo == SIGUSR1) {
    printf("catched user1\n");
    stop_player();
    hit_idx = -1;
    disp_curr_file();
  }
}

int main(int argc, char** argv)
{
  if(argc != 2) {
    printf("must specify target directory\n");
    return 1;
  }

  pid = getpid();
  printf("pid: %d\n", pid);

  player_bin = mug_query_config_string(CONFIG_PLAYER);
  player_option = mug_query_config_string(CONFIG_PLAYER_OPTION);
  vol_control = mug_query_config_string(CONFIG_VOL_CONTROL);
  kill_cmd = mug_query_config_string(CONFIG_KILL_CMD);
  run_cmd = mug_query_config_string(CONFIG_RUN_CMD);
  if(strlen(player_bin) == 0) {
    printf("must set %s in mug config file\n", CONFIG_PLAYER);
    return 1;
  }

  stop_player();

  target = argv[1];
  
  scan_files(target);

  signal(SIGUSR1, on_signal);

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
