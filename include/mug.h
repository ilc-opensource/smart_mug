#ifndef MUG_H
#define MUG_H

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

//#define USE_IOHUB

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
#define MUG_ASSERT(cond, info, ...) \
  if(!(cond)) {  \
    printf("%s:%d, %s\n", __FILE__, __LINE__, info); \
    assert(0);    \
  }

#ifndef _LIBIOHUB_H_
typedef long handle_t;

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

typedef struct _temp_data_t 
{
  int board_temp;
  int mug_temp;
  int battery_temp;
}temp_data_t;

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

#ifdef __cplusplus
extern "C" { 
#endif

typedef int mug_error_t;

#define MUG_ERROR_NONE 0

// device control
handle_t mug_init(device_t type);
void     mug_close(handle_t handle);

// display
handle_t     mug_disp_init();
mug_error_t  mug_disp_raw(handle_t handle, char* imgData);
mug_error_t  mug_disp_raw_N(handle_t handle, char* imgData, int number, int interval);
void         mug_stop_mcu_disp(handle_t handle);

// raw image buffer
char* mug_create_raw_buffer();
void  mug_free_raw_buffer(char *buf);

// image
int   mug_read_img(char *fname, char *buf);
int   mug_disp_img(handle_t handle, char *name); 
char* mug_read_img_N(char* names, int *num, int *size);
int   mug_disp_img_N(handle_t handle, char *names, int interval);

// cimg
int   mug_read_cimg(void *cimg, char *buf);
int   mug_disp_cimg(handle_t handle, void *cimg); 
void  mug_draw_number_cimg(void *img, int col, int row, char *str, unsigned char* color);
void  mug_number_text_shape(int *width, int *height);


// motion sensor
typedef struct _MPU6050 motion_data_t;
typedef void (*motion_cb_t)(int, int, int, int, int, int);

handle_t     mug_motion_init();
mug_error_t  mug_read_motion(handle_t handle, motion_data_t *data);
void         mug_motion_on(handle_t handle, motion_cb_t cb, int interval);
void         mug_run_motion_watcher(handle_t handle);


// temprature
typedef void (*temp_cb_t)(int, int, int);

handle_t    mug_temp_init();
mug_error_t mug_read_temp(handle_t handle, temp_data_t *data);
int         mug_read_board_temp(handle_t handle);
int         mug_read_mug_temp(handle_t handle);
int         mug_read_battery_temp(handle_t handle);
int         mug_temp_on(handle_t handle, temp_cb_t cb, int interval);
void        mug_run_temp_watcher(handle_t handle);


// touch panel
handle_t  mug_touch_init();
void      mug_touch_on(handle_t handle, touch_cb_t cb);
void      mug_gesture_on(handle_t handle, gesture_t g, gesture_cb_t cb);
void      mug_touch_event_on(handle_t handle, touch_event_t event, touch_event_cb_t cb);
void      mug_run_touch_thread(handle_t handle);
void      mug_stop_touch_thread(handle_t handle);
void      mug_wait_for_touch_thread(handle_t handle);

// utils
char*  get_proc_dir();

#ifdef __cplusplus
}
#endif

#endif
