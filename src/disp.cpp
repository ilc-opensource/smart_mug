#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <iohub_client.h>
#include <mug.h>
#include <res_manager.h>

#ifndef USE_IOHUB
#include <io.h>
#endif

struct __attribute__((packed)) led_line_data {
  uint8_t row;
  uint8_t reserved[2];
  uint8_t content[MAX_COLS/2];
};

error_t mug_disp_raw(handle_t handle, char* imgData) 
{
  int row, col;
  char *p = imgData;
  error_t err = ERROR_NONE;

  struct led_line_data data = {
    0, {0xff, 0xff}, {0}
  };

  for(row = 0; row < MAX_COMPRESSED_ROWS; row++) {

    // pack the data
    data.row = row;
    memcpy(&(data.content), p, MAX_COMPRESSED_COLS);

    // send to iohub
#ifdef USE_IOHUB
    err = iohub_send_command(handle, IOHUB_CMD_FB, (char*)&data, sizeof(data));
#else
    err = dev_send_command(handle, IOHUB_CMD_FB, (char*)&data, sizeof(data));
#endif
 
    if(err != ERROR_NONE) {
      MUG_ASSERT(0, "iohub_send_command error: %d\n", err);
      return err; 
    }

    // go to the next row
    p += MAX_COMPRESSED_COLS;  
  }

  return err;
}

error_t mug_disp_raw_N(handle_t handle, char* imgData, int number, int interval)
{
  int semResource = resource_init(LOCK_DISPLAY_TOUCH);
  char *p = imgData;
  error_t error = ERROR_NONE;
  int i;
  resource_wait(semResource);
  for(i = 0; i < number; i++) {
    error = mug_disp_raw(handle, p);

    if(error != ERROR_NONE) {
      printf("C program, disp page error!\n");
      fflush(NULL);
      resource_post(semResource);
      return error;
    }
    p += COMPRESSED_SIZE;
    usleep(interval * 1000);
  }
  resource_post(semResource);

  return error;
}

handle_t mug_init(device_t type) 
{
#ifdef USE_IOHUB
  handle_t handle = iohub_open_session(type);
#else
  handle_t handle = dev_open(type);
#endif
  return handle;
}

void mug_close(handle_t handle)
{
#ifdef USE_IOHUB
  iohub_close_session(handle);
#else
  dev_close(handle);
#endif
}

handle_t mug_disp_init()
{
  return mug_init(DEVICE_LED);
}
