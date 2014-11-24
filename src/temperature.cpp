#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <iohub_client.h>
#include <math.h>
#include <mug.h>
#include <RTC.h>
#include <battery.h>

#ifndef USE_IOHUB
#include <io.h>
#endif

#ifdef USE_LIBUV
#include <uv.h>
static uv_timer_t  temp_timer;
static uv_loop_t  *temp_loop = NULL;

typedef struct _req_temp_t {
  handle_t  handle;
  temp_cb_t temp_cb;
  battery_cb_t battery_cb;
} req_temp_t;

#endif

#define TEMP_NUM 3

int R_to_T(float r) 
{
  RT_t *rt;
  for(int i = 0; i < RT_table_len; i++) {
    rt = &(RT_table[i]);
    if(rt->rmin <= r && r <= rt->rmax)
      return rt->temp;
  }

  MUG_ASSERT(false, "can not calculate temperature\n");

}

float V_to_R(float v)
{
  return 10.0 * v /(3.3 - v);
}

int V_to_T(float v)
{
  float r = V_to_R(v);
  int t = R_to_T(r);
  return t;
}

handle_t mug_adc_init()
{
  return mug_init(DEVICE_LED);
}

handle_t mug_temp_init()
{
  return mug_init(DEVICE_LED);
}

handle_t mug_battery_init()
{
  return mug_init(DEVICE_LED);
}

int voltage_to_temp(uint16_t data)
{
#if 0
  float voltage = data * 3.3 / 1024;
  float tempf = 4050.0/(logf(0.213 * voltage / (3.3 - 2 * voltage)) + 13.59) - 273; 
  return (int)tempf;
#else
  float voltage = data * 3.3 / 1024;
  return V_to_T(voltage);
#endif
  
}

int voltage_to_percent(uint16_t data)
{
  V2P_t *v2p, *last;
  last = &(v2p_table[0]);

  int voltage = ((float)(data & 0x3ff)) * 3.0 * 1000 * 3 / (1024 * 2); 

  for(int i = 0; i < v2p_table_len; i++) {
    v2p = &(v2p_table[i]);

    if(voltage > v2p->v) 
      return last->percent;

    last = v2p;
  }
  return 0;
}

mug_error_t mug_read_adc(handle_t handle, adc_data_t *temp)
{
  uint16_t voltage[TEMP_NUM];
  memset(voltage, 0, sizeof(voltage));
#ifdef USE_IOHUB
  mug_error_t err = iohub_send_command(handle, IOHUB_CMD_ADC, (char*)&voltage, sizeof(voltage));
#else
  mug_error_t err = dev_send_command(handle, IOHUB_CMD_ADC, (char*)&voltage, sizeof(voltage));
#endif
  temp->mug_temp   = voltage_to_temp(voltage[0]);
  temp->board_temp  = voltage_to_temp(voltage[1]);
  temp->battery = voltage_to_percent(voltage[2]);
  return err;
}

int mug_read_board_temp(handle_t handle)
{
  adc_data_t data;
  mug_read_adc(handle, &data);
  return data.board_temp;
}

int mug_read_mug_temp(handle_t handle)
{
  adc_data_t data;
  mug_read_adc(handle, &data);
  return data.mug_temp;
}

int mug_read_battery(handle_t handle)
{
  adc_data_t data;
  mug_read_adc(handle, &data);
  return data.battery;
}

void run_adc_timer(uv_timer_t *req, int status)
{
  req_temp_t *rt = (req_temp_t*)(req->data);
  adc_data_t data;
  mug_read_adc(rt->handle, &data);

  if(rt->temp_cb != NULL)
    rt->temp_cb(data.mug_temp, data.board_temp);

  if(rt->battery_cb != NULL)
    rt->battery_cb(data.battery);
}

#ifdef USE_LIBUV

int mug_adc_on(handle_t handle, temp_cb_t temp_cb, battery_cb_t battery_cb, int interval)
{
  if(temp_loop == NULL) 
    temp_loop = uv_default_loop();

  uv_timer_init(temp_loop, &temp_timer);

  req_temp_t *req = (req_temp_t*)malloc(sizeof(req_temp_t));

  memset(req, sizeof(req_temp_t), 0);

  req->handle = handle;
  req->temp_cb = temp_cb;
  req->battery_cb = battery_cb;
  
  temp_timer.data = (void*)req;
  uv_timer_start(&temp_timer, run_adc_timer, 0, interval);
}

int mug_temp_on(handle_t handle, temp_cb_t cb, int interval)
{
  return mug_adc_on(handle, cb, NULL, interval);
}

int mug_battery_on(handle_t handle, battery_cb_t cb, int interval)
{
  return mug_adc_on(handle, NULL, cb, interval);
}

void mug_run_adc_watcher(handle_t handle)
{
  uv_run(temp_loop, UV_RUN_DEFAULT);
}

void mug_run_temp_watcher(handle_t handle)
{
  mug_run_adc_watcher(handle);
}

void mug_run_battery_watcher(handle_t handle)
{
  mug_run_adc_watcher(handle);
}

#endif
