#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <iohub_client.h>
#include <math.h>
#include <mug.h>
#include <RTC.h>

#include <config.h>

#include <list>
using namespace std;

#ifndef USE_IOHUB
#include <io.h>
#endif

#ifdef USE_LIBUV
#include <uv.h>
static uv_timer_t  temp_timer;
static uv_loop_t  *temp_loop = NULL;

// data type/structure for battery 
typedef struct _V2P_t {
  int percent;
  int v;
  int adc;
}V2P_t;

typedef list<V2P_t> v2p_table_t;

static v2p_table_t v2p_charging;
static v2p_table_t v2p_discharging;

// temperature adjustment table

typedef struct _temp_adjust_t {
  int temp;
  int adjust;
} temp_adjust_t;

typedef list<temp_adjust_t> temp_adjust_table_t;
static temp_adjust_table_t temp_adjust_table;

#define TEMP_NUM 3
#define MUG_TEMP_IDX 0
#define BOARD_TEMP_IDX 1
#define BATTERY_IDX 2

typedef int16_t adc_raw_t[3];

typedef struct _req_temp_t {
  handle_t  handle;
  temp_cb_t temp_cb;
  battery_cb_t battery_cb;
} req_temp_t;

#endif



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

void read_battery_table(v2p_table_t *table, char *type)
{
  char* t = mug_query_config_string(type);
  
  MUG_ASSERT(!(t == NULL || strlen(t) == 0), "can not find %s\n", type);

  FILE *fp = fopen(t, "r");
  MUG_ASSERT(fp != NULL, "can not open battery table: %s\n", t);
  
  V2P_t v2p;
  while(!feof(fp)) {
    fscanf(fp, "%d %d %d", &(v2p.percent), &(v2p.v), &(v2p.adc));
    table->push_back(v2p);
  }
}

void init_battery_table()
{
  read_battery_table(&v2p_charging,    CONFIG_CHARGE_TABLE);
  read_battery_table(&v2p_discharging, CONFIG_DISCHARGE_TABLE);
}

handle_t mug_battery_init()
{
  init_battery_table();
  return mug_init(DEVICE_LED);
}

void init_temp_adjust_table()
{
  char* t = mug_query_config_string(CONFIG_TEMP_ADJUST);  
  MUG_ASSERT(!(t == NULL || strlen(t) == 0), "can not find %s\n", CONFIG_TEMP_ADJUST);

  FILE *fp = fopen(t, "r");
  MUG_ASSERT(fp != NULL, "can not open battery table: %s\n", t);

  temp_adjust_t item;

  while(!feof(fp)) {
    fscanf(fp, "%d %d", &(item.temp), &(item.adjust));
    temp_adjust_table.push_back(item);
  }    
}

handle_t mug_temp_init()
{
  init_temp_adjust_table();
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
  int temp = V_to_T(voltage);

  if(temp_adjust_table.size() == 0)
    return temp;

  int modify = 0;

  temp_adjust_t last, item;
  
  last = temp_adjust_table.front();

  for(temp_adjust_table_t::iterator itr = temp_adjust_table.begin();
      itr != temp_adjust_table.end();
      itr++) {
    item = *itr;

    if(temp > item.temp)
      break;

    last = item;   
  }

  return temp + last.adjust;

#endif
  
}

int voltage_to_percent(uint16_t data, bool *is_charging)
{
  V2P_t v2p, last;

  *is_charging = ((data & 0x8000) != 0);
  //int voltage = ((float)(data & 0x3f0)) * 3.0 * 1000 * 3 / (1024 * 2); 
  int voltage = ((float)(data & 0x3ff)) * 3.0 * 1000 * 3 / (1024 * 2); 

  v2p_table_t *table;

  if(*is_charging) {
    table = &v2p_charging;
  } else {
    table = &v2p_discharging;
  }

  MUG_ASSERT(table->size() != 0, "Null battery voltage table\n");

  last = table->front();

  for(v2p_table_t::iterator itr = table->begin();
      itr != table->end();
      itr++) {

    v2p = *itr;
    if(voltage > v2p.v) 
      return last.percent;

    last = v2p;
  }

  return -1;
}

mug_error_t mug_read_adc(handle_t handle, adc_raw_t *data)
{

  memset(data, 0, sizeof(adc_raw_t));
#ifdef USE_IOHUB
  mug_error_t err = iohub_send_command(handle, IOHUB_CMD_ADC, (char*)data, sizeof(adc_raw_t));
#else
  mug_error_t err = dev_send_command(handle, IOHUB_CMD_ADC, (char*)data, sizeof(adc_raw_t));
#endif

  return err;
}

int mug_read_board_temp(handle_t handle)
{
  adc_raw_t raw;
  mug_read_adc(handle, &raw);
  return voltage_to_temp(raw[BOARD_TEMP_IDX]);
}

int mug_read_mug_temp(handle_t handle)
{
  adc_raw_t raw;
  mug_read_adc(handle, &raw);
  return voltage_to_temp(raw[MUG_TEMP_IDX]);
}

int mug_read_battery(handle_t handle, bool *is_charge)
{
  adc_raw_t raw;
  mug_read_adc(handle, &raw);
  
  return voltage_to_percent(raw[BATTERY_IDX], is_charge);
}

void run_adc_timer(uv_timer_t *req, int status)
{
  req_temp_t *rt = (req_temp_t*)(req->data);
  adc_raw_t raw;
  mug_read_adc(rt->handle, &raw);
  bool is_charging = true;
  int percent = 0;

  if(rt->temp_cb != NULL) {
    rt->temp_cb(voltage_to_temp(raw[MUG_TEMP_IDX]), voltage_to_temp(raw[BOARD_TEMP_IDX]));
  }

  if(rt->battery_cb != NULL) {
    percent = voltage_to_percent(raw[BATTERY_IDX], &is_charging);
    rt->battery_cb(percent, (int)is_charging); 
  }
}

#ifdef USE_LIBUV

int mug_adc_on(handle_t handle, temp_cb_t temp_cb, battery_cb_t battery_cb, int interval)
{
  if(temp_loop == NULL) 
    temp_loop = uv_default_loop();

  req_temp_t *req = (req_temp_t*)malloc(sizeof(req_temp_t));

  memset(req, 0, sizeof(req_temp_t));

  req->handle = handle;

  if(temp_cb != NULL)
    req->temp_cb = temp_cb;

  if(battery_cb != NULL) {
    req->battery_cb = battery_cb;
  }
  
  temp_timer.data = (void*)req;

  uv_timer_init(temp_loop, &temp_timer);
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
