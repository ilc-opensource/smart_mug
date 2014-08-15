#define __error_t_defined

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <fcntl.h>

#include <io.h>
#include <mug.h>

handle_t dev_open(device_t type)
{
  handle_t ret = 0;

  switch(type) {

  case DEVICE_LED:
  case DEVICE_ADC:
   ret = (handle_t)iohub_user_i2c_init();
   break;

  case DEVICE_MPU:
    ret = (handle_t)get_mpu_handle();
  break;

  case DEVICE_TP:
	  ret = (handle_t)get_tp_handle();
  break;
  return ret;
  }
}

void dev_close(handle_t handle)
{
  close((int)handle);
}

int read_with_timeout(handle_t handle, cmd_t cmdtype, char *data, int message_len)
{
  int hdl = (int)handle;

  fd_set set;
  struct timeval timeout;
  FD_ZERO(&set);
  FD_SET(hdl, &set);
  
  int rv;
  timeout.tv_sec = 0;
  timeout.tv_usec = 100 * 1000;
  
  rv = select(hdl + 1, &set, NULL, NULL, &timeout);

  if(rv > 0) {
    read(hdl, data, message_len);
  }

  return rv;
}

error_t dev_send_command(handle_t handle, cmd_t cmdtype, char *data, int message_len)
{
  error_t err = ERROR_NONE;
  ssize_t ret;

  switch(cmdtype){
  case IOHUB_CMD_ADC:
    err = (error_t)iohub_read_block_data((int)handle, cmdtype, message_len, (__u8*)data); 
    break;

  case IOHUB_CMD_FB:
    err = (error_t)iohub_write_block_data((int)handle, cmdtype, message_len, (const __u8*)data); 
    break;
  
  case IOHUB_CMD_MOTION_SENSOR:
    ret = read((int)handle, data, message_len);  
    if(ret == -1) 
      err = ERROR_NOT_AVAILABLE;
    break;

  case IOHUB_CMD_TOUCH_PANEL:
    ret = read_with_timeout(handle, cmdtype, data, message_len);
    if(ret <= 0)
      err = ERROR_CAN_NOT_GET_REPLY;
    break;
  }

  return err;
}


int get_mpu_handle()
{
  char node_path[MAX_STRING_SIZE];
  /* open data node */
  char module_name[MAX_STRING_SIZE] = "hwmon7";
  snprintf(node_path, MAX_STRING_SIZE, "/sys/class/hwmon/%s/device/data", module_name);
	
  int handle = open(node_path, O_RDONLY);
  MUG_ASSERT(handle != -1, "can not open mpu handle\n");

  return handle;
}

int get_tp_handle()
{
  char inputdevname[80]="/dev/input/event1";
  int handle = open(inputdevname, O_RDONLY);
  MUG_ASSERT(handle != -1, "can not open touch panel handle\n");
  return handle;
}

