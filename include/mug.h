#ifndef MUG_H
#define MUG_H

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

//#define USE_IOHUB

#define MUG_ENV "HOME"

#define MAX_ROWS 12
#define MAX_COLS 16
#define MAX_COMPRESSED_ROWS MAX_ROWS
#define MAX_COMPRESSED_COLS (MAX_COLS/2)
#define COMPRESSED_SIZE (MAX_COMPRESSED_ROWS * MAX_COMPRESSED_COLS)

#define TOUCH_WIDTH   800
#define TOUCH_HEIGHT  600
#define SCREEN_WIDTH  16
#define SCREEN_HEIGHT 12
#define TOUCH_WIDTH_SCALE  (TOUCH_WIDTH/SCREEN_WIDTH)
#define TOUCH_HEIGHT_SCALE (TOUCH_HEIGHT/SCREEN_HEIGHT)

#define DISP_INFINITE -1

#define MUG_ASSERT(cond, ...) \
  if(!(cond)) {  \
    printf("\nFATAL ERROR: "); \
    printf(__VA_ARGS__); \
    printf("\n"); \
    assert(0);    \
  }

#define MQ_NULL     0x0
#define MQ_PROLOG   0x1
#define MQ_EPILOG   0x2
#define MQ_ALL      0x3

typedef long handle_t;

#ifndef _LIBIOHUB_H_

typedef enum {
  DEVICE_INVALID = -1,
  DEVICE_LED,
  DEVICE_ADC,
  DEVICE_MPU,
  DEVICE_MAX,
} device_t;

struct _MPU6050 {
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t gx;
  int16_t gy;
  int16_t gz;
};

#endif

#define IMG_OK           0
#define IMG_ERROR        1

typedef struct _adc_data_t 
{
  int mug_temp;
  int board_temp;
  int battery;
}adc_data_t;

typedef struct _touch_point_t
{
  int x, y;
  int pressure;
  int tracking_id;
  int touch_major;
} touch_point_t;

typedef enum {
  MUG_NO_GESTURE = 0,
  
  MUG_GESTURE,

  MUG_SWIPE,
  MUG_SWIPE_LEFT,
  MUG_SWIPE_RIGHT,
  MUG_SWIPE_UP,
  MUG_SWIPE_DOWN,

  MUG_SWIPE_2,
  MUG_SWIPE_LEFT_2,
  MUG_SWIPE_RIGHT_2,
  MUG_SWIPE_UP_2,
  MUG_SWIPE_DOWN_2,

  MUG_GESTURE_NUM
}gesture_t;

typedef enum {
  TOUCH_EVENT_ALL = 0,

  TOUCH_CLICK,
  TOUCH_DOWN,
  TOUCH_UP,
  TOUCH_HOLD,

  TOUCH_EVENT_NUM
} touch_event_t;

typedef void (*gesture_cb_t)(gesture_t, char*); //gesture, info
typedef void (*touch_cb_t)(int, int, int); // x, y, id
typedef void (*touch_event_cb_t)(touch_event_t, int, int, int); // event, x, y, id

extern unsigned char red[3];
extern unsigned char green[3];
extern unsigned char blue[3];
extern unsigned char yellow[3];
extern unsigned char cyan[3];
extern unsigned char magenta[3];
extern unsigned char white[3];
extern unsigned char black[3];

typedef enum {
  RED,
  GREEN,
  BLUE,
  YELLOW,
  CYAN,
  MAGENTA,
  WHITE,
  BLACK
} mug_color_t;

#ifdef __cplusplus
extern "C" { 
#endif

typedef int mug_error_t;

#define MUG_ERROR_NONE 0

// device control
handle_t mug_init(device_t type);
void     mug_close(handle_t handle);
void     mug_shut_down_mcu(int sec);

// display
handle_t     mug_disp_init();
mug_error_t  mug_disp_raw(handle_t handle, char* imgData);
mug_error_t  mug_disp_raw_N(handle_t handle, char* imgData, int number, int interval);
void         mug_stop_mcu_disp(handle_t handle);

// raw image buffer
char* mug_create_raw_buffer();
void  mug_free_raw_buffer(char *buf);
void  mug_set_pixel_raw_color(char *raw, int col, int row, unsigned char color);
unsigned char * color_to_rgb(const char *color);


// raw image operation
int   mug_read_img_to_raw(char *fname, char *buf);
int   mug_disp_img(handle_t handle, char *name); 
char* mug_read_img_N(char* names, int *num, int *size);
int   mug_disp_img_N(handle_t handle, char *names, int interval);
void  mug_init_font(char *font);
void  mug_set_text_marquee_style(int s);
void  mug_disp_text_marquee(handle_t handle, const char *text, const char * color, int interval, int repeat);
void  mug_disp_text_marquee_async(handle_t handle, const char *text, const char * color, int interval, int repeat);

//color translation
unsigned char color_2_raw(const char* color);
unsigned char rgb_2_raw(unsigned char R,unsigned char G,unsigned B);

typedef unsigned long cimg_handle_t;

// cimg
int   mug_cimg_to_raw(cimg_handle_t cimg, char *buf);
int   mug_disp_cimg(handle_t handle, cimg_handle_t cimg); 
void  mug_number_text_shape(int *width, int *height);

// cimg handle

cimg_handle_t  mug_new_cimg(int width, int height);
cimg_handle_t  mug_new_canvas();
cimg_handle_t  mug_load_pic_cimg(char* fname);
void           mug_draw_number_str_cimg(cimg_handle_t img, int col, int row, const char *str, const char* color);
void           mug_overlay_cimg(cimg_handle_t canvas, int col, int row, cimg_handle_t img);
void           mug_destroy_cimg(cimg_handle_t hdl);
char*          mug_cimg_get_raw(cimg_handle_t cimg);
//void           mug_draw_text_cimg(cimg_handle_t img, int col, int row, const char *text, const char* color, int height);
void           mug_draw_text_cimg(cimg_handle_t img, int col, int row, const char *text, const char* color, int height, int *str_width, int *str_height);
void           mug_save_cimg(cimg_handle_t cimg, char *name);
void           mug_disp_cimg_marquee(handle_t handle, cimg_handle_t img, int interval, int repeat, int seamless = MQ_PROLOG | MQ_EPILOG);
void           mug_disp_cimg_marquee_async(handle_t handle, cimg_handle_t img, int interval, int repeat, int seamless = MQ_PROLOG | MQ_EPILOG);
void           mug_stop_marquee(handle_t handle);

cimg_handle_t  mug_new_text_cimg(const char* text, const char* color);

// motion sensor
typedef struct _MPU6050 motion_data_t;
typedef void (*motion_cb_t)(int, int, int, int, int, int);
typedef void (*motion_angel_cb_t)(float, float, float);
typedef void (*motion_shake_cb_t)(int);

handle_t     mug_motion_init();
mug_error_t  mug_read_motion(handle_t handle, motion_data_t *data);
void         mug_motion_on(handle_t handle, motion_cb_t cb);
void         mug_motion_angle_on(handle_t handle, motion_angel_cb_t acb);
void         mug_set_motion_timer(handle_t handle, int interval);
void         mug_motion_shake_on(handle_t handle, motion_shake_cb_t scb);
void         mug_config_shake(handle_t handle, int period, int times);
void         mug_run_motion_watcher(handle_t handle);


typedef void (*temp_cb_t)(int, int);
typedef void (*battery_cb_t)(int, int);

// adc
handle_t    mug_adc_init();
int         mug_adc_on(handle_t handle, temp_cb_t temp_cb, battery_cb_t battery_cb, int interval);
void        mug_run_adc_watcher(handle_t handle);

// temprature
handle_t    mug_temp_init();
int         mug_read_board_temp(handle_t handle);
int         mug_read_mug_temp(handle_t handle);
int         mug_temp_on(handle_t handle, temp_cb_t cb, int interval);
void        mug_run_temp_watcher(handle_t handle);

// battery
handle_t    mug_battery_init();
int         mug_read_battery(handle_t handle, bool *is_charge);
int         mug_battery_on(handle_t handle, battery_cb_t cb, int interval);
void        mug_run_battery_watcher(handle_t handle);



// touch panel
handle_t  mug_touch_init();
void      mug_touch_on(handle_t handle, touch_cb_t cb);
void      mug_gesture_on(handle_t handle, gesture_t g, gesture_cb_t cb);
void      mug_touch_event_on(handle_t handle, touch_event_t event, touch_event_cb_t cb);
void      mug_run_touch_thread(handle_t handle);
void      mug_stop_touch_thread(handle_t handle);
void      mug_wait_for_touch_thread(handle_t handle);

// configuration
int         mug_query_config_int(const char *key);
double      mug_query_config_double(const char *key);
const char* mug_query_config_string(const char *key);

// utils
char*  get_proc_dir();

#ifdef __cplusplus
}
#endif

#endif
