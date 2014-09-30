#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "session.h"

#define MAX_STRING_SIZE 255
#define MAX_DEVICE_INDEX 25
#define MAX_UNSIGNED_INT 0xffffffff

/* todo: adapative device numbers */
typedef enum {
  DEVICE_INVALID = -1,
  DEVICE_LED,
  DEVICE_ADC,
  DEVICE_MPU,
  DEVICE_TP,
  DEVICE_ADS1198,
  DEVICE_MAX,
} device_t;

static char device_names[DEVICE_MAX][DEVICE_NAME_MAX] = {
  {"LED"},
  {"ADC"},
  {"MPU"},
  {"TP"},
  {"ADS1198"},
};

//device related message

/* data structure to keep state for a particular type of device*/
typedef struct {
  unsigned char device_id;
  char name[DEVICE_NAME_MAX+1];
  session_state_t *list;
} device_state_t;

typedef struct _MPU6050 {
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t gx;
  int16_t gy;
  int16_t gz;
} MPU6050;

#include <linux/input.h>

#endif
