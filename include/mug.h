#ifndef MUG_H
#define MUG_H

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define MAX_ROWS 12
#define MAX_COLS 16
#define MAX_COMPRESSED_ROWS MAX_ROWS
#define MAX_COMPRESSED_COLS (MAX_COLS/2)
#define COMPRESSED_SIZE (MAX_COMPRESSED_ROWS * MAX_COMPRESSED_COLS)

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

/*error message*/
typedef enum {
  ERROR_NONE = 0,
  ERROR_NOT_AVAILABLE = -1,
  ERROR_MESSAGE_NOT_SENT = -2,
  ERROR_CAN_NOT_GET_REPLY = -3,
} error_t;
#endif

#define IMG_OK           0
#define IMG_ERROR        1

#ifdef __cplusplus
extern "C" { 
#endif

// device control
handle_t mug_init(device_t type);
void     mug_close(handle_t handle);

// display
handle_t mug_disp_init();
error_t  mug_disp_raw(handle_t handle, char* imgData);
error_t  mug_disp_raw_N(handle_t handle, char* imgData, int number, int interval);

// raw image buffer
char* mug_create_raw_buffer();
void  mug_free_raw_buffer(char *buf);

// image
int   mug_read_img(char *fname, char *buf);
int   mug_disp_img(handle_t handle, char* name); 

char* mug_read_img_N(char* names, int *num);
int   mug_disp_img_N(handle_t handle, char *names, int interval);

// motion sensor
typedef struct _MPU6050 motion_data_t;
typedef void (*motion_cb_t)(motion_data_t *motion);

handle_t mug_motion_sensor_init();
error_t  mug_read_motion_sensor(handle_t handle, motion_data_t *data);
void     mug_read_motion_sensor_async(handle_t handle, motion_cb_t cb);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class Mug {
public: 
  Mug();
  ~Mug();
  int dispRaw(char* data, int number = 1, int interval = 40);
  int dispImgs(char* names, int interval = 40);
  error_t readMotion(motion_data_t *data);
private:
  handle_t *m_handles;
  int       m_handleNum;

  handle_t  initHandle(device_t type);
  void      closeHandles();

  handle_t  getHandle(device_t type);
};
#endif

#endif
