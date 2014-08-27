#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <iohub_client.h>
#include <math.h>
#include <mug.h>

#ifndef USE_IOHUB
#include <io.h>
#endif

#ifdef USE_LIBUV
#include <uv.h>
static uv_timer_t  temp_timer;
static uv_loop_t  *temp_loop = NULL;

typedef struct _req_temp_t {
  handle_t  handle;
  temp_cb_t cb;
} req_temp_t;

#endif

#define TEMP_NUM 3
handle_t mug_temp_init()
{
  return mug_init(DEVICE_LED);
}

int voltage_to_temp(uint16_t data)
{
  float voltage = data * 3.3 / 1024;
  float tempf = 4050.0/(logf(0.213 * voltage / (3.3 - 2 * voltage)) + 13.59) - 273; 
  return (int)tempf;
}

void mug_read_temp(handle_t handle, temp_data_t *temp)
{
  uint16_t voltage[TEMP_NUM];
  memset(voltage, 0, sizeof(voltage));
#ifdef USE_IOHUB
  error_t err = iohub_send_command(handle, IOHUB_CMD_ADC, (char*)&voltage, sizeof(voltage));
#else
  error_t err = dev_send_command(handle, IOHUB_CMD_ADC, (char*)&voltage, sizeof(voltage));
#endif
  temp->board_temp   = voltage_to_temp(voltage[0]);
  temp->mug_temp     = voltage_to_temp(voltage[1]);
  temp->battery_temp = voltage_to_temp(voltage[2]);
}

int mug_read_board_temp(handle_t handle)
{
  temp_data_t data;
  mug_read_temp(handle, &data);
  return data.board_temp;
}

int mug_read_mug_temp(handle_t handle)
{
  temp_data_t data;
  mug_read_temp(handle, &data);
  return data.mug_temp;
}

int mug_read_battery_temp(handle_t handle)
{
  temp_data_t data;
  mug_read_temp(handle, &data);
  return data.battery_temp;
}

void run_temp_timer(uv_timer_t *req, int status)
{
  req_temp_t *rt = (req_temp_t*)(req->data);
  temp_data_t data;
  mug_read_temp(rt->handle, &data);
  rt->cb(data.board_temp, data.mug_temp, data.battery_temp);
}

#ifdef USE_LIBUV

int mug_temp_on(handle_t handle, temp_cb_t cb, int interval)
{
  if(temp_loop == NULL) 
    temp_loop = uv_default_loop();

  uv_timer_init(temp_loop, &temp_timer);

  req_temp_t *req = (req_temp_t*)malloc(sizeof(req_temp_t));

  req->handle = handle;
  req->cb = cb;
  
  temp_timer.data = (void*)req;
  uv_timer_start(&temp_timer, run_temp_timer, 0, interval);
}

void mug_run_temp_watcher(handle_t handle)
{
  uv_run(temp_loop, UV_RUN_DEFAULT);
}

#endif
