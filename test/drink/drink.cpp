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

#include <cJSON.h>

#define CONFIG "drink_config.json"
#define PREFIX "drink"
#define DIR    "drink_trace"

#define CONFIG_START_TIME    "start_time"
#define CONFIG_TRIGGER_DEG   "trigger_deg"
#define CONFIG_MIN_DEG       "min_deg"
#define CONFIG_DUR_TIME      "dur_time"
#define CONFIG_TRACE         "trace"

#define MAX_SIZE (2 * 1024 * 1024)
#define START_TIME 0
#define LEAST_TIME 2

static bool disp = false;
static bool touch = false;

typedef list<time_t> time_list_t;

typedef struct _trace_summary_t {
    int count;
    time_t last;
    time_t first;
} trace_summary_t;

static trace_summary_t summary;

typedef struct _config_t {
    time_t start_time;
    int trigger_deg;
    int min_deg;
    int dur_time;
    string trace_file;
} config_t;

static config_t config;

typedef list<int> degree_list_t;

class trace_t {
public:
    trace_t();
    int id;
    degree_list_t degree_list;
    bool catch_it;
    int drink_deg;
    int min_deg;
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
    if(!disp) return;
    
    clear_canvas();
    canvas.draw_rectangle(0, 0, 0,
                          SCREEN_WIDTH/4, SCREEN_HEIGHT/4, 0,
                          green);
    mug_disp_cimg(disp_handle, (cimg_handle_t)&canvas);
}

void disp_stop()
{
    if(!disp) return;
    
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
    fprintf(f, "    \"id\"       : %d,\n", trace.id);
    fprintf(f, "    \"drink_deg\": %d,\n", trace.drink_deg);
    fprintf(f, "    \"min_deg\"  : %d,\n", trace.min_deg);
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

time_t parse_time(char *str)
{
    struct tm when;
    memset(&when, 0, sizeof(when));
    char *ret = strptime(str, "%Y-%m-%d %H:%M:%S", &when);
    MUG_ASSERT(ret != NULL, "can not parse time: %s\n", str);
    
    return mktime(&when);
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
    
    cJSON *json = cJSON_Parse(data);
    
    free(data);
    
    // check size
    cJSON *item = cJSON_GetObjectItem(json, CONFIG_START_TIME);
    
    if(item != NULL)
        config.start_time = parse_time(item->valuestring);
    
    item = cJSON_GetObjectItem(json, CONFIG_TRIGGER_DEG);
    MUG_ASSERT(item, "please set %s", CONFIG_TRIGGER_DEG);
    config.trigger_deg = item->valueint;
    
    item = cJSON_GetObjectItem(json, CONFIG_MIN_DEG);
    MUG_ASSERT(item, "please set %s", CONFIG_MIN_DEG);
    config.min_deg = item->valueint;
    
    item = cJSON_GetObjectItem(json, CONFIG_DUR_TIME);
    MUG_ASSERT(item, "please set %s", CONFIG_DUR_TIME);
    config.dur_time = item->valueint;
 
    item = cJSON_GetObjectItem(json, CONFIG_TRACE);
    MUG_ASSERT(item, "please set %s", CONFIG_TRACE);
    config.trace_file = item->valuestring;
    
}

void write_trace(time_t when)
{
    FILE * fp = fopen(config.trace_file.c_str(), "a");
    MUG_ASSERT(fp != NULL, "can not open %s to write trace time\n", config.trace_file.c_str());
    
    fprintf(fp, "%d\n", when);
    
    fclose(fp);
}

void init_trace()
{
    FILE *fp = fopen(config.trace_file.c_str(), "r");
    time_list_t collects;
    
    if(fp != NULL) {
        
        
        time_t when;

        
        while(fscanf(fp, "%d", &when) != EOF) {
	
            printf("%d\n",when);
            if(when >= config.start_time)
                collects.push_back(when);
        }
        
        fclose(fp);
    }
    
    
    summary.count = collects.size();
    
    if(summary.count > 0) {
        summary.first = collects.front();
        summary.last = collects.back();
    }
    
    printf("summary: %d\n", summary.count);
    
    fp = fopen(config.trace_file.c_str(), "w+");
    MUG_ASSERT(fp != NULL, "can not open %s to write\n", config.trace_file.c_str());
    
    for(time_list_t::iterator itr = collects.begin();
        itr != collects.end();
        itr++) {
        fprintf(fp, "%d\n", *itr);
    }
    
    fclose(fp);
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
    
    FILE *fp = fopen(temp, "w+");
    
    MUG_ASSERT(fp != NULL, "can not open %s", temp);
    
    fprintf(fp, "{\n");
    
}

void clear_trace()
{
    trace.degree_list.clear();
    trace.drink_deg = 0;
    trace.dur_time = 0;
    trace.is_drinking = false;
    trace.min_deg = 90;
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
    
    if(trace.dur_time < config.dur_time) {
        printf("abondon trace due to duration time %d < %d\n",trace.dur_time, config.dur_time);
        clear_trace();
        return;
    }
    
    if(trace.min_deg > config.min_deg) {
        printf("abondon trace due to min_deg %d > %d\n", trace.min_deg, config.min_deg);
        clear_trace();
        return;
    }
    
    summary.count++;
    summary.last = trace.end_time;
    //dump_trace(fp);
    dump_trace(stdout);
    write_trace(trace.end_time);
}

void on_signal(int signo)
{
    printf("captured ctrl + c\n");
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
    
    if(angle_z <= config.trigger_deg) {
        if(!trace.is_drinking) {
            printf("start drink, when > %d\n", config.trigger_deg);
            start_trace();
        }
    } else {
        if(trace.is_drinking) {
            printf("stop drink\n");
            stop_trace();
        }
    }
    
    if(trace.is_drinking) {
        
        if((int)angle_z < trace.min_deg)
            trace.min_deg = (int)angle_z;
        
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

    read_config();
    init_trace();
    
    signal(SIGINT, on_signal);
    
    if(disp) {
        disp_handle = mug_disp_init();
    }
    
    if(touch) {
        touch_handle = mug_touch_init();
        mug_touch_event_on(touch_handle, TOUCH_EVENT_ALL, on_touch);
    }
    
    motion_handle = mug_motion_init();
    mug_motion_angle_on(motion_handle, on_motion_angle);
    mug_set_motion_timer(motion_handle, 200);
    mug_run_motion_watcher(motion_handle);
    
    return 0;
}
