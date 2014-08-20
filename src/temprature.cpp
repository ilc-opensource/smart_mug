#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <iohub_client.h>
#include <math.h>
#include <mug.h>

#ifndef USE_IOHUB
#include <io.h>
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

