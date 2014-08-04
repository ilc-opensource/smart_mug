#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <iohub_client.h>
#include <mug.h>

struct __attribute__((packed)) led_line_data {
  uint8_t row;
  uint8_t reserved[2];
  uint8_t content[MAX_COLS/2];
};

static void it8350_read_adc(handle_t handle)
{
  uint16_t data[3];
  memset(data, 0, sizeof(data));
  uint8_t i;

  error_t err = iohub_send_command(handle, IOHUB_CMD_ADC, (char*)&data, sizeof(data));

  if(err) {
    fprintf(stderr, "read ADC failed\n");
    return;
  }
  fprintf(stdout, "ADC:");
  for(i = 0; i < sizeof (data)/sizeof(uint16_t); i++) {
     fprintf (stdout, " 0x%02x%02x", (uint8_t)(data[i]>>8), (uint8_t)data[i]);
  }
  fprintf(stdout, "\n");
}

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
    err = iohub_send_command(handle, IOHUB_CMD_FB, (char*)&data, sizeof(data));  
    if(err != ERROR_NONE)
      return err; 

    // go to the next row
    p += MAX_COMPRESSED_COLS;  
  }

  // check status
  it8350_read_adc(handle);

  return err;
}

error_t mug_disp_raw_N(handle_t handle, char* imgData, int number, int interval)
{
  char *p = imgData;
  error_t error = ERROR_NONE;
  int i;
  for(i = 0; i < number; i++) {
    error = mug_disp_raw(handle, p);

    if(error != ERROR_NONE) {
      printf("error!");
      return error;
    }
    p += COMPRESSED_SIZE;
    usleep(interval * 1000);
  }

  return error;
}

handle_t mug_init(device_t type) 
{
  handle_t handle = iohub_open_session(type);
  return handle;
}

void mug_close(handle_t handle)
{
  iohub_close_session(handle);
}

handle_t mug_disp_init()
{
  return mug_init(DEVICE_LED);
}
