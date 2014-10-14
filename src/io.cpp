#define __error_t_defined

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <io.h>
#include <mug.h>


#define TP_DEV_PATH         "/dev/input/event1"
#define MPU_DEV_PATH        "/sys/class/hwmon/hwmon7/device/data"

handle_t dev_open(device_t type)
{
  handle_t ret = 0;

  switch(type) {

  case DEVICE_LED:
  case DEVICE_ADC:
   ret = (handle_t)iohubd_user_i2c_init();
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

  if(rv < 0) {
    printf("select err: %d, %s\n", errno, strerror(errno));
  }

  if(rv > 0) {
    read(hdl, data, message_len);
  }

  return rv;
}

mug_error_t dev_send_command(handle_t handle, cmd_t cmdtype, char *data, int message_len)
{
  mug_error_t err = ERROR_NONE;
  ssize_t ret;

  switch(cmdtype){
  case IOHUB_CMD_ADC:
    err = (mug_error_t)iohubd_read_block_data((int)handle, cmdtype, message_len, (__u8*)data); 
    break;

  case IOHUB_CMD_FB:
    err = (mug_error_t)iohubd_write_block_data((int)handle, cmdtype, message_len, (const __u8*)data); 
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
  int handle = open(MPU_DEV_PATH, O_RDONLY);

  MUG_ASSERT(handle != -1, "can not open mpu handle\n");

  return handle;
}

int get_tp_handle()
{
  int handle = open(TP_DEV_PATH, O_RDONLY);

  MUG_ASSERT(handle != -1, "can not open touch panel handle\n");

  return handle;
}

int LedFrame_Set(int fd, BOOL flags, BYTE frameId)
{
  struct LedFrameMesg mesg;

  mesg.type = FRAME_CMD_SET;
  mesg.frame = frameId;
  mesg.set.flags = flags;

  return iohubd_write_block_data(fd, IOHUB_CMD_FRAME, offsetof(struct LedFrameMesg, set) + sizeof(mesg.set), (const __u8 *)&mesg);
}

int LedFrame_Set_Row(int fd, BYTE frameId, BYTE rowId, const void *content)
{
  struct LedFrameMesg mesg;

  mesg.type = FRAME_CMD_ROW;
  mesg.frame = frameId;

  mesg.row.index = rowId;
  memcpy(mesg.row.content, content, sizeof(mesg.row.content));
  return iohubd_write_block_data(fd, IOHUB_CMD_FRAME, sizeof(mesg), (const __u8 *)&mesg);
}

int LedFrame_Set_Duration(int fd, BYTE frameId, DWORD ms)
{
  struct LedFrameMesg mesg;

  mesg.type = FRAME_CMD_SET;
  mesg.frame = frameId;
  mesg.set.flags = FRAME_DURATION_MASK;
  mesg.set.duration = ms;

  return iohubd_write_block_data(fd, IOHUB_CMD_FRAME, offsetof(struct LedFrameMesg, set) + sizeof(mesg.set), (const __u8 *)&mesg);
}

void stop_mcu_disp(int fd, int cnt)
{
  LedFrame_Set(fd, FRAME_ACTIVE_MASK, 0);

  sleep(2);

  BYTE content[BYTES_PER_ROW];

  /* update frames */
  for (BYTE frameId = 0; (frameId < cnt) && (frameId < MAX_LED_FRAMES); frameId++) {
    LedFrame_Set(fd, FRAME_ENABLE_MASK, frameId);
    for (BYTE rowId = 0; rowId < MAX_ROWS; rowId++) {
      memset (content, (frameId + rowId) & 0x77, sizeof(content));
      LedFrame_Set_Row(fd, frameId, rowId, content);
    }
    LedFrame_Set_Duration(fd, frameId, 500);
  }
}

void mug_stop_mcu_disp(handle_t handle)
{
  stop_mcu_disp(handle, 40);
}
